#pragma once

#include "template/noncopyable.h"
#include "core/vec.h"
#include "tiny_obj_loader.h"

class Material;
class StaticMesh;

struct OBJModel
{
	OBJModel()
		: staticMesh(nullptr)
		, minBound(vec3(0.0f, 0.0f, 0.0f))
		, maxBound(vec3(0.0f, 0.0f, 0.0f))
	{
	}

	StaticMesh* staticMesh;
	vec3 minBound;
	vec3 maxBound;
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
