#pragma once

#include "hit.h"
#include "src/material.h"

class sphere : public Hitable
{

public:
	sphere(const vec3& inCenter, float inRadius, Material* inMaterial)
		: center(inCenter)
		, radius(inRadius)
		, material(inMaterial)
	{
	}

	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& result) const;

	virtual bool BoundingBox(float t0, float t1, AABB& outBox) const override;

	vec3 center;
	float radius;
	Material* material;

};
