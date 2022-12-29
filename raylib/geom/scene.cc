#include "scene.h"
#include "bvh.h"

Scene::~Scene()
{
	if (accelStruct) delete accelStruct;
}

void Scene::AddSceneElement(Hitable* hitable)
{
	if (!bFinalized)
	{
		hitableList.hitables.push_back(hitable);
	}
}

BVHNode* Scene::Finalize()
{
	if (!bFinalized)
	{
		bFinalized = true;
		accelStruct = new BVHNode(&hitableList, 0.0f, 0.0f);
	}
	return accelStruct;
}
