#pragma once

#include "raylib_types.h"
#include "geom/hit.h"

class Material;

class Cube : public Hitable
{
public:
	static Cube FromMinMaxBounds(const vec3& inMinBounds, const vec3& inMaxBounds, float inTimeStartMove, vec3 inVelocity, Material* inMaterial)
	{
		return Cube(inMinBounds, inMaxBounds, inTimeStartMove, inVelocity, inMaterial);
	}
	static Cube FromOriginAndExtent(const vec3& inOrigin, const vec3& inExtent, float inTimeStartMove, vec3 inVelocity, Material* inMaterial)
	{
		return Cube(inOrigin - inExtent, inOrigin + inExtent, inTimeStartMove, inVelocity, inMaterial);
	}

public:
	Cube(const vec3& inMinBounds, const vec3& inMaxBounds, float inTimeStartMove, vec3 inVelocity, Material* inMaterial)
		: minBounds(inMinBounds)
		, maxBounds(inMaxBounds)
		, timeStartMove(inTimeStartMove)
		, velocity(inVelocity)
		, material(inMaterial)
	{
	}

	Cube() : Cube(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 0.0f, vec3(0.0f, 0.0f, 0.0f), nullptr) {}

	RAYLIB_API virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const;

	RAYLIB_API virtual bool BoundingBox(float t0, float t1, AABB& outBox) const override;

public:
	vec3 minBounds;
	vec3 maxBounds;
	float timeStartMove;
	vec3 velocity;
	Material* material;
};
