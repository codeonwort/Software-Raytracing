#include "scene.h"
#include "bvh.h"

Scene::Scene()
{
	sunIlluminance = vec3(0.0f);
	sunDirection = normalize(vec3(0.0f, -1.0f, -0.5f));
}

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
