#pragma once

#include "../template/noncopyable.h"
#include "tiny_obj_loader.h"

class OBJLoader : public Noncopyable
{

public:
	static void Initialize();
	static void Destroy();

	static bool SyncLoad(const char* filepath);

public:
	explicit OBJLoader();
	~OBJLoader();

	bool LoadSynchronous(const char* filepath);

	/*
	 * #todo: async load
	 * ThreadHandle LoadAsync(std::vector<const char*> filepathArray);
	 * AsyncLoadProgress GetProgress();
	 */

private:
	tinyobj::ObjReader internalLoader;

};
