#pragma once

#include "raylib_types.h"
#include "geom/cube.h"
#include "geom/triangle.h"
#include "geom/transform.h"

class BVHNode;

class StaticMesh : public Hitable
{
public:
	RAYLIB_API StaticMesh() {}
	~StaticMesh();

	RAYLIB_API void AddTriangle(const Triangle& triangle);

	// Call this if you already know the bounds, otherwise CalculateBounds().
	RAYLIB_API void SetBounds(const AABB& inBounds);

	// Call this if you don't know the bounds, otherwise SetBounds().
	RAYLIB_API void CalculateBounds();

	RAYLIB_API void ApplyTransform(const Transform& transform);

	// Lock modification and build acceleration structure
	RAYLIB_API void Finalize();

	RAYLIB_API virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const override;

	RAYLIB_API virtual bool BoundingBox(float t0, float t1, AABB& outBox) const override;

private:
	std::vector<Triangle> triangles;

	// conservative bounds
	AABB bounds;
	bool boundsValid = false;

	BVHNode* bvh = nullptr;

	bool bLocked = false;
};
