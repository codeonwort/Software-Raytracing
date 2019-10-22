#include "obj_loader.h"
#include "src/type.h"
#include "src/log.h"
#include "src/util/resource_finder.h"
#include "src/geom/static_mesh.h"
#include "src/geom/triangle.h"
#include "src/material.h"


void OBJLoader::Initialize()
{
	log("Initialize obj loader");
}

void OBJLoader::Destroy()
{
	log("Destroy obj loader");
}

bool OBJLoader::SyncLoad(const char* filepath, OBJModel& outModel)
{
	OBJLoader loader;
	bool bLoaded = loader.LoadSynchronous(filepath, outModel);
	// #todo: create obj model
	return bLoaded;
}

OBJLoader::OBJLoader()
{
}

OBJLoader::~OBJLoader()
{
	// #todo: cancel async load
}

bool OBJLoader::LoadSynchronous(const char* filepath, OBJModel& outModel)
{
	std::string file = ResourceFinder::Get().Find(filepath);
	if (file.size() == 0)
	{
		return false;
	}

	if (!internalLoader.ParseFromFile(file))
	{
		return false;
	}

	const tinyobj::attrib_t& attrib = internalLoader.GetAttrib();
	const std::vector<tinyobj::shape_t>& shapes = internalLoader.GetShapes();
	const std::vector<tinyobj::material_t>& materials = internalLoader.GetMaterials();

	log("%s: load %s", __FUNCTION__, filepath);
	log("\tvertices: %d", (int32)(attrib.vertices.size() / 3));
	log("\tmaterials: %d", (int32)materials.size());

	StaticMesh* mesh = new StaticMesh;
	vec3 minBound(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
	vec3 maxBound(-FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX);

	// #todo: Parse material info
	Lambertian* temp_material = new Lambertian(vec3(1.0f, 0.2f, 0.2f));

	for (const tinyobj::shape_t& shape : shapes)
	{
		// mesh
		int32 p = 0;
		int32 numFaces = (int32)shape.mesh.num_face_vertices.size();
		for (int32 f = 0; f < numFaces; ++f)
		{
			int32 fv = shape.mesh.num_face_vertices[f];
			if (fv != 3)
			{
				// #todo: deal with non-triangle
				log("%s: (%s) %d-th face is non-triangle", __FUNCTION__, shape.name.data(), f);
				continue;
			}
			int32 i0 = (int32)shape.mesh.indices[p++].vertex_index;
			int32 i1 = (int32)shape.mesh.indices[p++].vertex_index;
			int32 i2 = (int32)shape.mesh.indices[p++].vertex_index;
			vec3 v0(attrib.vertices[i0 * 3], attrib.vertices[i0 * 3 + 1], attrib.vertices[i0 * 3 + 2]);
			vec3 v1(attrib.vertices[i1 * 3], attrib.vertices[i1 * 3 + 1], attrib.vertices[i1 * 3 + 2]);
			vec3 v2(attrib.vertices[i2 * 3], attrib.vertices[i2 * 3 + 1], attrib.vertices[i2 * 3 + 2]);

			minBound = min(min(min(minBound, v0), v1), v2);
			maxBound = max(max(max(maxBound, v0), v1), v2);

			v0 *= 0.07f;
			v1 *= 0.07f;
			v2 *= 0.07f;

			mesh->AddTriangle(Triangle(v0, v1, v2, temp_material));
		}

		// #todo: lines
		// #todo: points
	}

	outModel.staticMesh = mesh;
	outModel.minBound = minBound;
	outModel.maxBound = maxBound;

	log("\t> loading done");

	return true;
}
