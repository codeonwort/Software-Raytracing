#pragma once

#include "hit.h"
#include "src/material.h"

class Triangle : public Hitable
{

public:
	Triangle(const vec3& inV0, const vec3& inV1, const vec3& inV2, Material* inMaterial)
		: v0(inV0)
		, v1(inV1)
		, v2(inV2)
		, material(inMaterial)
	{
		UpdateNormal();
	}

	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const;

	void GetVertices(vec3& outV0, vec3& outV1, vec3& outV2) const;
	void SetVertices(const vec3& inV0, const vec3& inV1, const vec3& inV2);

private:
	inline void UpdateNormal()
	{
		n = cross(v1 - v0, v2 - v0);
		n.Normalize();
	}

	vec3 v0;
	vec3 v1;
	vec3 v2;
	vec3 n;
	Material* material;
	
};
