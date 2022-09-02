#include "obj_loader.h"
#include "image_loader.h"
#include "type.h"
#include "log.h"
#include "image.h"
#include "core/vec.h"
#include "geom/static_mesh.h"
#include "geom/triangle.h"
#include "geom/bvh.h"
#include "shading/material.h"
#include "util/resource_finder.h"

// -------------------------------
// OBJModel

void OBJModel::FinalizeAllMeshes() {
	for (StaticMesh* mesh : staticMeshes) {
		mesh->Finalize();
	}
}

// -------------------------------
// OBJLoader

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
		log("%s: File not exist: %s", __FUNCTION__, filepath);
		return false;
	}

	// Call tiny_obj_loader.
	if (!internalLoader.ParseFromFile(file))
	{
		log("%s: tiny_obj_loader failed: %s", __FUNCTION__, filepath);
		return false;
	}

	const tinyobj::attrib_t& attrib = internalLoader.GetAttrib();
	const std::vector<tinyobj::shape_t>& shapes = internalLoader.GetShapes();
	const std::vector<tinyobj::material_t>& raw_materials = internalLoader.GetMaterials();

	if (shapes.size() == 0) {
		log("%s: No shapes found in: %s", __FUNCTION__, filepath);
		return false;
	}

	log("%s: Load %s", __FUNCTION__, filepath);
	log("\tTotal shapes: %d", (int32)shapes.size());
	log("\tTotal vertices: %d", (int32)(attrib.vertices.size() / 3));
	log("\tTotal materials: %d", (int32)raw_materials.size());

	// Used for faces whose materials are not specified.
	Lambertian* const fallbackMaterial = new Lambertian(vec3(0.5f, 0.5f, 0.5f));

	log("Parsing materials...");
	ParseMaterials(filepath, raw_materials, materials);

	vec3 localMinBound(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
	vec3 localMaxBound(-FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX);
	for (auto i = 0u; i < attrib.vertices.size(); i += 3) {
		vec3 v(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);
		localMinBound = min(localMinBound, v);
		localMaxBound = max(localMaxBound, v);
	}

	const bool hasNormals = (attrib.normals.size() > 0) && (attrib.normals.size() == attrib.vertices.size());

	// Convert each tinyobj::shape_t into a StaticMesh.
	log("Parsing shapes...");
	int32 shapeIx = 0;
	for (const tinyobj::shape_t& shape : shapes)
	{
		log("\tParsing shape %d: %s", shapeIx++, shape.name.c_str() ? shape.name.c_str() : "<noname>");

		// Temp storages for current shape.
		std::vector<Triangle> triangles;
		std::vector<vec3> customNormals(attrib.vertices.size(), vec3(0.0f, 0.0f, 0.0f));
		std::vector<int32> customNormalIndices(attrib.vertices.size() * 3, -1);

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

			vec3 n0, n1, n2;
			if (hasNormals)
			{
				n0 = vec3(attrib.normals[i0 * 3], attrib.normals[i0 * 3 + 1], attrib.normals[i0 + 3 + 2]);
				n1 = vec3(attrib.normals[i1 * 3], attrib.normals[i1 * 3 + 1], attrib.normals[i1 + 3 + 2]);
				n2 = vec3(attrib.normals[i2 * 3], attrib.normals[i2 * 3 + 1], attrib.normals[i2 + 3 + 2]);
			}
			else
			{
				vec3 n = normalize(cross(v1 - v0, v2 - v0));
				customNormals[i0] += n;
				customNormals[i1] += n;
				customNormals[i2] += n;
				customNormalIndices[p] = i0;
				customNormalIndices[p + 1] = i1;
				customNormalIndices[p + 2] = i2;
			}

			Material* faceMaterial = fallbackMaterial;
			int32 material_ix = shape.mesh.material_ids[f];
			if (0 <= material_ix && material_ix < materials.size() && materials[material_ix] != nullptr)
			{
				faceMaterial = materials[material_ix];
			}

			Triangle T(v0, v1, v2, n0, n1, n2, faceMaterial);

			i0 = (int32)shape.mesh.indices[p].texcoord_index;
			i1 = (int32)shape.mesh.indices[p + 1].texcoord_index;
			i2 = (int32)shape.mesh.indices[p + 2].texcoord_index;
			if (i0 != -1 && i1 != -1 && i2 != -1) {
				T.SetParameterization(
					attrib.texcoords[i0 * 2], 1.0f - attrib.texcoords[i0 * 2 + 1],
					attrib.texcoords[i1 * 2], 1.0f - attrib.texcoords[i1 * 2 + 1],
					attrib.texcoords[i2 * 2], 1.0f - attrib.texcoords[i2 * 2 + 1]);
			} else {
				T.SetParameterization(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
			}

			triangles.emplace_back(T);
			p += 3;
		}

		// #todo-obj: lines
		// #todo-obj: points

		if (!hasNormals) {
			for (auto i = 0; i < customNormals.size(); ++i) {
				customNormals[i] = normalize(customNormals[i]);
			}
			for (auto i = 0; i < triangles.size(); ++i) {
				const vec3& n0 = customNormals[customNormalIndices[i * 3]];
				const vec3& n1 = customNormals[customNormalIndices[i * 3 + 1]];
				const vec3& n2 = customNormals[customNormalIndices[i * 3 + 2]];
				triangles[i].SetNormals(n0, n1, n2);
			}
		}

		StaticMesh* mesh = new StaticMesh;
		for (const Triangle& T : triangles) {
			mesh->AddTriangle(T);
		}
		mesh->CalculateBounds();

		outModel.staticMeshes.push_back(mesh);
	}

	if (outModel.staticMeshes.size() == 1) {
		outModel.rootObject = outModel.staticMeshes[0];
	} else {
		std::vector<Hitable*> hitables;
		for (StaticMesh* mesh : outModel.staticMeshes) {
			hitables.push_back(mesh);
		}
		outModel.rootObject = new BVHNode(new HitableList(hitables), 0.0f, 0.0f);
		//outModel.rootObject = new HitableList(hitables);
	}

	outModel.localMinBound = localMinBound;
	outModel.localMaxBound = localMaxBound;

	log("> OBJ loading done");

	return true;
}

void OBJLoader::ParseMaterials(const std::string& objpath, const std::vector<tinyobj::material_t>& inRawMaterials, std::vector<Material*>& outMaterials)
{
	int32 nMaterials = (int32)inRawMaterials.size();
	outMaterials.resize(nMaterials, nullptr);

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

	for (int32 i = 0; i < nMaterials; ++i)
	{
		const tinyobj::material_t& rawMaterial = inRawMaterials[i];

		log("\tParsing material %d: %s", i, rawMaterial.name.c_str() ? rawMaterial.name.c_str() : "<noname>");

		// #todo-obj: Parse every data in the material
		// PBR extension in tiny_obj_loader.h
#if 0
		real_t roughness;            // [0, 1] default 0
		real_t metallic;             // [0, 1] default 0
		real_t sheen;                // [0, 1] default 0
		real_t clearcoat_thickness;  // [0, 1] default 0
		real_t clearcoat_roughness;  // [0, 1] default 0
		real_t anisotropy;           // aniso. [0, 1] default 0
		real_t anisotropy_rotation;  // anisor. [0, 1] default 0
		real_t pad0;
		std::string roughness_texname;  // map_Pr
		std::string metallic_texname;   // map_Pm
		std::string sheen_texname;      // map_Ps
		std::string emissive_texname;   // map_Ke
		std::string normal_texname;     // norm. For normal mapping.
#endif

		Image2D albedoImage;
		Image2D roughnessImage;
		Image2D metallicImage;
		Image2D emissiveImage;
		Image2D normalImage;

		bool albedoImageValid;
		bool roughnessImageValid;
		bool metallicImageValid;
		bool emissiveImageValid;
		bool normalImageValid;

		auto LoadImage = [&basedir](const std::string& inFilename, Image2D& outImage, bool& outValid)
		{
			if (inFilename.size() == 0)
			{
				outValid = false;
				return;
			}
			std::string filepath = ResourceFinder::Get().Find(basedir + inFilename);
			outValid = ImageLoader::SyncLoad(filepath.data(), outImage);
		};

		LoadImage(rawMaterial.diffuse_texname, albedoImage, albedoImageValid);
		LoadImage(rawMaterial.roughness_texname, roughnessImage, roughnessImageValid);
		LoadImage(rawMaterial.metallic_texname, metallicImage, metallicImageValid);
		LoadImage(rawMaterial.emissive_texname, emissiveImage, emissiveImageValid);
		LoadImage(rawMaterial.normal_texname, normalImage, normalImageValid);

		MicrofacetMaterial* M = new MicrofacetMaterial;

		if (albedoImageValid) M->SetAlbedoTexture(albedoImage);
		if (normalImageValid) M->SetNormalTexture(normalImage);
		if (roughnessImageValid) M->SetRoughnessTexture(roughnessImage);
		if (metallicImageValid) M->SetMetallicTexture(metallicImage);
		if (emissiveImageValid) M->SetEmissiveTexture(emissiveImage);

		M->SetAlbedoFallback(vec3(rawMaterial.diffuse[0], rawMaterial.diffuse[1], rawMaterial.diffuse[2]));
		// #todo-obj: tinyobjloader's roughness defaults to 0.0 if not specified, but it means every materials gonna be specular.
		// And there is no way to check if roughness was just omitted or explicitly set to zero :(
		if (rawMaterial.roughness > 0.0f) {
			M->SetRoughnessFallback(rawMaterial.roughness);
		} else {
			// #todo-pbr: Invalid result if roughness is too low.
			constexpr float tempMinRoughness = 0.01f;
			// Ad-hoc derivation of roughness from specular
			float avgSpec = (1.0f / 3.0f) * (rawMaterial.specular[0] + rawMaterial.specular[1] + rawMaterial.specular[2]);
			float fakeRoughness = std::max(tempMinRoughness, 1.0f - avgSpec);
			M->SetRoughnessFallback(fakeRoughness);
		}
		M->SetMetallicFallback(rawMaterial.metallic);
		M->SetEmissiveFallback(vec3(rawMaterial.emission[0], rawMaterial.emission[1], rawMaterial.emission[2]));

		outMaterials[i] = M;
	}
}
