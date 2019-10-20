#include "obj_loader.h"
#include "src/log.h"
#include "src/util/resource_finder.h"


void OBJLoader::Initialize()
{
	log("Initialize obj loader");
}

void OBJLoader::Destroy()
{
	log("Destroy obj loader");
}

bool OBJLoader::SyncLoad(const char* filepath)
{
	OBJLoader loader;
	bool bLoaded = loader.LoadSynchronous(filepath);
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

bool OBJLoader::LoadSynchronous(const char* filepath)
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

	return true;
}
