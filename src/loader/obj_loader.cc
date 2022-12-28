#include "obj_loader.h"
#include "image_loader.h"
#include "core/int_types.h"
#include "core/vec3.h"
#include "geom/static_mesh.h"
#include "geom/triangle.h"
#include "geom/bvh.h"
#include "render/material.h"
#include "render/image.h"
#include "util/log.h"

/* 'illum' cheat sheet
0. Color on and Ambient off
1. Color on and Ambient on
2. Highlight on
3. Reflection on and Ray trace on
4. Transparency: Glass on, Reflection: Ray trace on
5. Reflection: Fresnel on and Ray trace on
6. Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
7. Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
8. Reflection on and Ray trace off
9. Transparency: Glass on, Reflection: Ray trace off
10. Casts shadows onto invisible surfaces
*/

// Albedo = 1.0 is physically impossible.
#define MAX_ALBEDO vec3(0.95f)

vec3 ToVec3(const tinyobj::real_t tiny_real[3])
{
	return vec3(tiny_real[0], tiny_real[1], tiny_real[2]);
}

// https://learn.microsoft.com/en-us/azure/remote-rendering/reference/material-mapping
float PhongSpecularToRoughness(const vec3& specularPower, float shininess)
{
	float intensity = (specularPower.x + specularPower.y + specularPower.z) / 3.0f;
	return std::sqrt(2.0f / (shininess * intensity + 2.0f));
}

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
	LOG("Initialize obj loader");
}

void OBJLoader::Destroy()
{
	LOG("Destroy obj loader");
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
	if (filepath == nullptr)
	{
		LOG("%s: filepath was null", __FUNCTION__);
		return false;
	}

	// Call tiny_obj_loader.
	if (!internalLoader.ParseFromFile(filepath))
	{
		LOG("%s: tiny_obj_loader failed: %s", __FUNCTION__, filepath);
		return false;
	}

	const tinyobj::attrib_t& tiny_attrib = internalLoader.GetAttrib();
	const std::vector<tinyobj::shape_t>& tiny_shapes = internalLoader.GetShapes();
	const std::vector<tinyobj::material_t>& tiny_materials = internalLoader.GetMaterials();

	if (tiny_shapes.size() == 0)
	{
		LOG("%s: No shapes found in: %s", __FUNCTION__, filepath);
		return false;
	}

	LOG("%s: Load %s", __FUNCTION__, filepath);
	LOG("\tTotal shapes: %d", (int32)tiny_shapes.size());
	LOG("\tTotal vertices: %d", (int32)(tiny_attrib.vertices.size() / 3));
	LOG("\tTotal materials: %d", (int32)tiny_materials.size());

	// Used for faces whose materials are not specified.
	Lambertian* const fallbackMaterial = new Lambertian(vec3(0.5f, 0.5f, 0.5f));

	LOG("Parsing %u materials...", tiny_materials.size());
	PreloadImages(filepath, tiny_materials);
	ParseMaterials(filepath, tiny_materials, materials);

	vec3 localMinBound(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
	vec3 localMaxBound(-FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX);
	for (auto i = 0u; i < tiny_attrib.vertices.size(); i += 3)
	{
		vec3 v(tiny_attrib.vertices[i], tiny_attrib.vertices[i + 1], tiny_attrib.vertices[i + 2]);
		localMinBound = min(localMinBound, v);
		localMaxBound = max(localMaxBound, v);
	}

	// Convert each tinyobj::shape_t into a StaticMesh.
	LOG("Parsing %u shapes...", tiny_shapes.size());
	int32 shapeIx = 0;
	int32 DEBUG_numInvalidTexcoords = 0;

	for (const tinyobj::shape_t& shape : tiny_shapes)
	{
		//LOG("\tParsing shape %d: %s", shapeIx++, shape.name.c_str() ? shape.name.c_str() : "<noname>");

		// Temp storages for current shape.
		std::vector<Triangle> triangles;

		int32 numFaces = (int32)shape.mesh.num_face_vertices.size();
		size_t index_offset = 0;

		for (int32 faceID = 0; faceID < numFaces; ++faceID)
		{
			int32 numFaceVertices = shape.mesh.num_face_vertices[faceID];
			if (numFaceVertices != 3)
			{
				// #todo-obj: deal with non-triangle
				LOG("%s: (%s) %d-th face is non-triangle", __FUNCTION__, shape.name.data(), faceID);
				continue;
			}

			vec3 positions[3], normals[3];
			float texUs[3], texVs[3];
			bool bValidNormal = true;
			for (int32 faceVertexID = 0; faceVertexID < numFaceVertices; ++faceVertexID)
			{
				tinyobj::index_t idx = shape.mesh.indices[index_offset + faceVertexID];

				// position data (should exist)
				float posX = tiny_attrib.vertices[3 * idx.vertex_index + 0];
				float posY = tiny_attrib.vertices[3 * idx.vertex_index + 1];
				float posZ = tiny_attrib.vertices[3 * idx.vertex_index + 2];

				// texcoord data (optional)
				float texU = 0.0f;
				float texV = 0.0f;
				if (idx.texcoord_index >= 0)
				{
					// UV wrapping should be performed at texture sampling, not here.
					texU = tiny_attrib.texcoords[2 * idx.texcoord_index + 0];
					texV = tiny_attrib.texcoords[2 * idx.texcoord_index + 1];
				}
				else
				{
					++DEBUG_numInvalidTexcoords;
				}

				// normal data (optional)
				float nx = 0.0f, ny = 0.0f, nz = 0.0f;
				if (idx.normal_index >= 0)
				{
					nx = tiny_attrib.normals[3 * idx.normal_index + 0];
					ny = tiny_attrib.normals[3 * idx.normal_index + 1];
					nz = tiny_attrib.normals[3 * idx.normal_index + 2];
				}
				else
				{
					bValidNormal = false;
				}

				positions[faceVertexID] = vec3(posX, posY, posZ);
				texUs[faceVertexID] = texU;
				texVs[faceVertexID] = texV;
				normals[faceVertexID] = vec3(nx, ny, nz);
			}

			// Well let's not try to smooth out the face normals...
			if (!bValidNormal)
			{
				vec3 n = cross(positions[1] - positions[0], positions[2] - positions[0]);
				normals[0] = normals[1] = normals[2] = normalize(n);
			}

			Material* faceMaterial = fallbackMaterial;
			int32 material_ix = shape.mesh.material_ids[faceID];
			if (0 <= material_ix && material_ix < materials.size() && materials[material_ix] != nullptr)
			{
				faceMaterial = materials[material_ix];
			}

			Triangle T(
				positions[0], positions[1], positions[2],
				normals[0], normals[1], normals[2],
				faceMaterial);

			T.SetParameterization(texUs[0], texVs[0], texUs[1], texVs[1], texUs[2], texVs[2]);

			triangles.emplace_back(T);

			index_offset += numFaceVertices;
		}

		// #todo-obj: lines
		// #todo-obj: points

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

	if (DEBUG_numInvalidTexcoords > 0)
	{
		LOG("WARNING: Num triangles with invalid UVs: %d", DEBUG_numInvalidTexcoords);
	}
	LOG("> OBJ loading done");

	return true;
}

void OBJLoader::PreloadImages(
	const std::string& objpath,
	const std::vector<tinyobj::material_t>& tinyMaterials)
{
	auto LoadImage = [&objpath, &imageDB = this->imageDB](const std::string& inFilename)
	{
		if (inFilename.size() > 0 && imageDB.find(inFilename) == imageDB.end())
		{
			std::string basedir;
			if (objpath.find_last_of("/\\") != std::string::npos)
			{
				basedir = objpath.substr(0, objpath.find_last_of("/\\") + 1);
				std::string filepath = basedir + inFilename;

				std::shared_ptr<Image2D> image = std::make_shared<Image2D>();
				ImageLoader::SyncLoad(filepath.data(), *image);
				imageDB.insert(std::make_pair(inFilename, image));
			}
		}
	};

	for (const tinyobj::material_t& material : tinyMaterials)
	{
		LoadImage(material.diffuse_texname);
		LoadImage(material.roughness_texname);
		LoadImage(material.metallic_texname);
		LoadImage(material.emissive_texname);

		// #todo-obj: In tinyobjlaoder, are bump texture and normal texture same thing?
		LoadImage(material.normal_texname);
		LoadImage(material.bump_texname);
	}

	LOG("\t%u image files has been loaded", imageDB.size());
}

void OBJLoader::ParseMaterials(
	const std::string& objpath,
	const std::vector<tinyobj::material_t>& inRawMaterials,
	std::vector<Material*>& outMaterials)
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

		//LOG("\tParsing material %d: %s", i, rawMaterial.name.c_str() ? rawMaterial.name.c_str() : "<noname>");

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

		auto FindImage = [&imageDB = this->imageDB](const std::string& filename) {
			auto it = imageDB.find(filename);
			if (it != imageDB.end())
			{
				return it->second;
			}
			return std::shared_ptr<Image2D>();
		};

		std::shared_ptr<Image2D> albedoImage = FindImage(rawMaterial.diffuse_texname);
		std::shared_ptr<Image2D> roughnessImage = FindImage(rawMaterial.roughness_texname);
		std::shared_ptr<Image2D> metallicImage = FindImage(rawMaterial.metallic_texname);
		std::shared_ptr<Image2D> emissiveImage = FindImage(rawMaterial.emissive_texname);
		
		std::shared_ptr<Image2D> normalImage = FindImage(rawMaterial.normal_texname);
		if (normalImage == nullptr) normalImage = FindImage(rawMaterial.bump_texname);

		const vec3 albedoConstant = min(MAX_ALBEDO, ToVec3(rawMaterial.diffuse));

		bool bTransparentIllum = (rawMaterial.illum == 4 || rawMaterial.illum == 6);
		bool bZeroDiffuse = (rawMaterial.diffuse_texname.size() == 0) && (albedoConstant == vec3(0.0f));
		// Ad-hoc conditions as some opaque models have illum 4
		if (bTransparentIllum && bZeroDiffuse)
		{
			vec3 transmittance = ToVec3(rawMaterial.transmittance);
			Dielectric* M = new Dielectric(rawMaterial.ior, transmittance);
			outMaterials[i] = M;
		}
		else if (rawMaterial.illum == 3)
		{
			Mirror* M = new Mirror(albedoConstant);
			outMaterials[i] = M;
		}
		else
		{
			MicrofacetMaterial* M = new MicrofacetMaterial;

			if (albedoImage) M->SetAlbedoTexture(albedoImage);
			if (normalImage) M->SetNormalTexture(normalImage);
			if (roughnessImage) M->SetRoughnessTexture(roughnessImage);
			if (metallicImage) M->SetMetallicTexture(metallicImage);
			if (emissiveImage) M->SetEmissiveTexture(emissiveImage);

			M->SetAlbedoFallback(albedoConstant);

			// tinyobjloader's roughness defaults to 0.0 if not specified.
			// And there is no way to check if roughness was just omitted or explicitly set to zero.
			// -> Use roughness if non-zero. Otherwise, derive it from Phong parameters.
			if (rawMaterial.roughness > 0.0f) {
				M->SetRoughnessFallback(rawMaterial.roughness);
			} else {
				vec3 specularPower = ToVec3(rawMaterial.specular);
				float fakeRoughness = PhongSpecularToRoughness(specularPower, rawMaterial.shininess);
				M->SetRoughnessFallback(fakeRoughness);
			}

			M->SetMetallicFallback(rawMaterial.metallic);
			M->SetEmissiveFallback(ToVec3(rawMaterial.emission));

			outMaterials[i] = M;
		}
	}
}
