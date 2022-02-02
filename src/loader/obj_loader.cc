#include "obj_loader.h"
#include "src/type.h"
#include "src/log.h"
#include "src/util/resource_finder.h"
#include "src/geom/static_mesh.h"
#include "src/geom/triangle.h"
#include "src/material.h"
#include "src/image.h"
#include "image_loader.h"


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
	return bLoaded;
}

OBJLoader::OBJLoader()
{
}

OBJLoader::~OBJLoader()
{
	// #todo-obj: cancel async load
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
	const std::vector<tinyobj::material_t>& raw_materials = internalLoader.GetMaterials();

	log("%s: load %s", __FUNCTION__, filepath);
	log("\tvertices: %d", (int32)(attrib.vertices.size() / 3));
	log("\tmaterials: %d", (int32)raw_materials.size());

	// #todo-obj: Parse material info
	Lambertian* fallbackMaterial = new Lambertian(vec3(1.0f, 0.2f, 0.2f));
	ParseMaterials(filepath, raw_materials, materials);

	StaticMesh* mesh = new StaticMesh;
	vec3 minBound(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
	vec3 maxBound(-FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX);

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
				// #todo-obj: deal with non-triangle
				log("%s: (%s) %d-th face is non-triangle", __FUNCTION__, shape.name.data(), f);
				continue;
			}
			int32 i0 = (int32)shape.mesh.indices[p].vertex_index;
			int32 i1 = (int32)shape.mesh.indices[p + 1].vertex_index;
			int32 i2 = (int32)shape.mesh.indices[p + 2].vertex_index;
			vec3 v0(attrib.vertices[i0 * 3], attrib.vertices[i0 * 3 + 1], attrib.vertices[i0 * 3 + 2]);
			vec3 v1(attrib.vertices[i1 * 3], attrib.vertices[i1 * 3 + 1], attrib.vertices[i1 * 3 + 2]);
			vec3 v2(attrib.vertices[i2 * 3], attrib.vertices[i2 * 3 + 1], attrib.vertices[i2 * 3 + 2]);

			// #todo-obj: Just min/max attrib.vertices. We're checking same vertices again and again here.
			minBound = min(min(min(minBound, v0), v1), v2);
			maxBound = max(max(max(maxBound, v0), v1), v2);

			Material* faceMaterial = fallbackMaterial;
			int32 material_ix = shape.mesh.material_ids[f];
			if (0 <= material_ix && material_ix < materials.size() && materials[material_ix] != nullptr)
			{
				faceMaterial = materials[material_ix];
			}

			Triangle T(v0, v1, v2, faceMaterial);

			i0 = (int32)shape.mesh.indices[p].texcoord_index;
			i1 = (int32)shape.mesh.indices[p + 1].texcoord_index;
			i2 = (int32)shape.mesh.indices[p + 2].texcoord_index;
			T.SetParameterization(
				attrib.texcoords[i0 * 2], 1.0f - attrib.texcoords[i0 * 2 + 1],
				attrib.texcoords[i1 * 2], 1.0f - attrib.texcoords[i1 * 2 + 1],
				attrib.texcoords[i2 * 2], 1.0f - attrib.texcoords[i2 * 2 + 1]);

			mesh->AddTriangle(T);

			p += 3;
		}

		// #todo-obj: lines
		// #todo-obj: points
	}

	outModel.staticMesh = mesh;
	outModel.minBound = minBound;
	outModel.maxBound = maxBound;

	outModel.staticMesh->SetBounds(AABB(minBound, maxBound));

	log("\t> loading done");

	return true;
}

void OBJLoader::ParseMaterials(const std::string& objpath, const std::vector<tinyobj::material_t>& inRawMaterials, std::vector<Material*>& outMaterials)
{
	int32 n = (int32)inRawMaterials.size();
	outMaterials.resize(n, nullptr);

	std::string basedir;
	if (objpath.find_last_of("/\\") != std::string::npos)
	{
		basedir = objpath.substr(0, objpath.find_last_of("/\\") + 1);
	}
	else
	{
		// #todo-obj: error handling
		CHECK(false);
	}

	for (int32 i = 0; i < n; ++i)
	{
		// #todo-obj: Parse every data in the material
		std::string albedoName = inRawMaterials[i].diffuse_texname;
		if (albedoName.size() == 0)
		{
			continue;
		}

		std::string albedoFile = ResourceFinder::Get().Find(basedir + albedoName);

		Image2D albedoImage;
		if (ImageLoader::SyncLoad(albedoFile.data(), albedoImage))
		{
			outMaterials[i] = new TextureMaterial(albedoImage);
		}
		else
		{
			// #todo-obj: error handling
			CHECK(false);
		}
	}
}
