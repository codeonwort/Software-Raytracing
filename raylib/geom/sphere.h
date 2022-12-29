#pragma once

#include "raylib_types.h"
#include "geom/hit.h"

class Material;

class Sphere : public Hitable
{
public:
	Sphere(const vec3& inCenter, float inRadius, Material* inMaterial)
		: center(inCenter)
		, radius(inRadius)
		, material(inMaterial)
	{
	}

	RAYLIB_API virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& result) const;

	RAYLIB_API virtual bool BoundingBox(float t0, float t1, AABB& outBox) const override;

public:
	vec3 center;
	float radius;
	Material* material;
};
