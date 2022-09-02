#pragma once

#include "template/noncopyable.h"
#include "core/vec.h"

#include "tiny_obj_loader.h"

#include <vector>

class Material;
class StaticMesh;
class Hitable;

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

	/*
	 * #todo-obj: async load
	 * ThreadHandle LoadAsync(std::vector<const char*> filepathArray);
	 * AsyncLoadProgress GetProgress();
	 */

private:
	void ParseMaterials(const std::string& objpath, const std::vector<tinyobj::material_t>& inRawMaterials, std::vector<Material*>& outMaterials);

	std::vector<Material*> materials;
	tinyobj::ObjReader internalLoader;

};
