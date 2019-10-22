#pragma once

#include "src/template/noncopyable.h"
#include "tiny_obj_loader.h"

class StaticMesh;

struct OBJModel
{
	OBJModel()
		: staticMesh(nullptr)
	{
	}

	StaticMesh* staticMesh;
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
	 * #todo: async load
	 * ThreadHandle LoadAsync(std::vector<const char*> filepathArray);
	 * AsyncLoadProgress GetProgress();
	 */

private:
	tinyobj::ObjReader internalLoader;

};
