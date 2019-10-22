#pragma once

#include "triangle.h"

// #todo-staticmesh: acceleration structure (maybe a simple octree will work fine)
class StaticMesh : public Hitable
{

public:
	StaticMesh()
	{
	}

	void AddTriangle(const Triangle& triangle);

	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const override;

private:
	std::vector<Triangle> triangles;

};
