#pragma once

#include "raylib_types.h"
#include "geom/hit.h"

class BVHNode : public Hitable
{
	
public:
	RAYLIB_API BVHNode(HitableList* list, float t0, float t1);

	RAYLIB_API virtual bool Hit(const ray& r, float tMin, float tMax, HitResult& outResult) const override;

	RAYLIB_API virtual bool BoundingBox(float t0, float t1, AABB& outBox) const override;

private:
	BVHNode(Hitable** list, int32 n, float t0, float t1);

public:
	Hitable* left = nullptr;
	Hitable* right = nullptr;
	AABB box;
};
