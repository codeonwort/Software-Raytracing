#pragma once

#include "hit.h"

class Material;

class Triangle : public Hitable
{

public:
	Triangle(const vec3& inV0, const vec3& inV1, const vec3& inV2, Material* inMaterial)
		: v0(inV0)
		, v1(inV1)
		, v2(inV2)
		, s0(0.0f), t0(0.0f), s1(0.0f), t1(0.0f), s2(0.0f), t2(0.0f)
		, material(inMaterial)
	{
		UpdateNormal();
	}

	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const;

	inline void SetParameterization(float inS0, float inT0, float inS1, float inT1, float inS2, float inT2)
	{
		s0 = inS0; t0 = inT0;
		s1 = inS1; t1 = inT1;
		s2 = inS2; t2 = inT2;
	}
	inline void GetParameterization(float& outS0, float& outT0, float& outS1, float& outT1, float& outS2, float& outT2) const
	{
		outS0 = s0; outT0 = t0;
		outS1 = s1; outT1 = t1;
		outS2 = s2; outT2 = t2;
	}

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

	// Surface parameterization
	// Oops. v for vertex :(
	// Let's use 's' and 't'
	float s0, t0, s1, t1, s2, t2; 
	
	Material* material;
	
};
