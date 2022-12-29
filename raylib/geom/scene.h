// Very thin scene abstraction for renderer.

#pragma once

#include "raylib_types.h"
#include "hit.h"
#include "bvh.h"

class Scene
{
public:
	Scene() = default;
	~Scene();

	void AddSceneElement(Hitable* hitable);

	BVHNode* Finalize();

	inline const BVHNode* GetAccelStruct() const { return accelStruct; }

private:
	HitableList hitableList;
	BVHNode* accelStruct = nullptr;
	bool bFinalized = false;
};
