#pragma once

#include "core/noncopyable.h"
#include "core/vec3.h"

#include <vector>
#include <memory>
#include "tiny_obj_loader.h"

class Material;
class StaticMesh;
class Hitable;
class Image2D;

// Parsing result of a Wavefront OBJ file.
// CAUTION: You shoul finalize static meshes by StaticMesh::Finalize(),
//          or you can just call OBJModel::FinalizeAllMeshes().
struct OBJModel
{
	OBJModel()
		: rootObject(nullptr)
		, localMinBound(vec3(0.0f, 0.0f, 0.0f))
		, localMaxBound(vec3(0.0f, 0.0f, 0.0f))
		
	{
	}

	void FinalizeAllMeshes();

	Hitable* rootObject;
	std::vector<StaticMesh*> staticMeshes;

	// CAUTION: These will become invalid after applying transforms to the meshes.
	vec3 localMinBound;
	vec3 localMaxBound;
};

class OBJLoader : public Noncopyable
{

public:
	static void Initialize();
	static void Destroy();

	static bool SyncLoad(const char* filepath, OBJModel& outModel);

public:
	explicit OBJLoader();
	~OBJLoader();

	bool LoadSynchronous(const char* filepath, OBJModel& outModel);

private:
	void PreloadImages(
		const std::string& objpath,
		const std::vector<tinyobj::material_t>& tinyMaterials);

	void ParseMaterials(
		const std::string& objpath,
		const std::vector<tinyobj::material_t>& inRawMaterials,
		std::vector<Material*>& outMaterials);

	std::map<std::string, std::shared_ptr<Image2D>> imageDB;
	std::vector<Material*> materials;
	tinyobj::ObjReader internalLoader;

};
