#pragma once

#include "raylib_types.h"
#include "geom/hit.h"

class Material;

class Triangle : public Hitable
{

public:
	RAYLIB_API Triangle(
		const vec3& inV0, const vec3& inV1, const vec3& inV2,
		const vec3& inN0, const vec3& inN1, const vec3& inN2,
		Material* inMaterial);

	RAYLIB_API virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const;

	RAYLIB_API virtual bool BoundingBox(float t0, float t1, AABB& outBox) const override;

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

	RAYLIB_API void GetVertices(vec3& outV0, vec3& outV1, vec3& outV2) const;
	RAYLIB_API void SetVertices(const vec3& inV0, const vec3& inV1, const vec3& inV2);

	RAYLIB_API void GetNormals(vec3& outN0, vec3& outN1, vec3& outN2) const;
	RAYLIB_API void SetNormals(const vec3& inN0, const vec3& inN1, const vec3& inN2);

private:
	inline void UpdateNormal()
	{
		n = cross(v1 - v0, v2 - v0);
		n.Normalize();
	}

	vec3 v0;
	vec3 v1;
	vec3 v2;

	// Perp to this triangle; auto-derived
	vec3 n;
	// Assigned to each vertex and interpolated by barycentric
	vec3 n0;
	vec3 n1;
	vec3 n2;

	AABB bounds;

	// Surface parameterization
	float s0, t0, s1, t1, s2, t2; 
	
	Material* material;
	
};
