#pragma once

#include "cube.h"
#include "triangle.h"
#include "src/transform.h"

class BVHNode;

class StaticMesh : public Hitable
{

public:
	StaticMesh() {}
	~StaticMesh();

	void AddTriangle(const Triangle& triangle);

	// Call this if you already know the bounds, otherwise CalculateBounds().
	void SetBounds(const AABB& inBounds);

	// Call this if you don't know the bounds, otherwise SetBounds().
	void CalculateBounds();

	void ApplyTransform(const Transform& transform);

	// Lock modification and build acceleration structure
	void Finalize();

	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const override;

	virtual bool BoundingBox(float t0, float t1, AABB& outBox) const override;

private:
	std::vector<Triangle> triangles;

	// conservative bounds
	AABB bounds;
	bool boundsValid = false;

	BVHNode* bvh = nullptr;

	bool bLocked = false;

};
