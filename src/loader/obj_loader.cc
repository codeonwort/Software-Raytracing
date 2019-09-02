#include "obj_loader.h"
#include "../log.h"


void OBJLoader::Initialize()
{
	log("Initialize obj loader");
}

void OBJLoader::Destroy()
{
	log("Destroy obj loader");
}

void OBJLoader::SyncLoad(const char* filepath)
{
	OBJLoader loader;
	loader.LoadSynchronous(filepath);
}

OBJLoader::OBJLoader()
{
}

OBJLoader::~OBJLoader()
{
	// #todo: cancel async load
}

void OBJLoader::LoadSynchronous(const char* filepath)
{
	internalLoader.ParseFromFile(filepath);
}
