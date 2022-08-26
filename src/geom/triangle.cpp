#include "triangle.h"
#include "shading/material.h"

// http://geomalgorithms.com/a06-_intersect-2.html
bool Triangle::Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const
{
	float paramU, paramV;

	float t = dot((v0 - r.o), n) / dot(r.d, n);
	vec3 p = r.o + t * r.d;

	if (t < t_min || t > t_max)
	{
		return false;
	}

	vec3 u = v1 - v0;
	vec3 v = v2 - v0;
	vec3 w = p - v0;

	float uv = dot(u, v);
	float wv = dot(w, v);
	float uu = dot(u, u);
	float vv = dot(v, v);
	float wu = dot(w, u);
	float uvuv = uv * uv;
	float uuvv = uu * vv;

	paramU = (uv * wv - vv * wu) / (uvuv - uuvv);
	paramV = (uv * wu - uu * wv) / (uvuv - uuvv);

	if (0.0f <= paramU && 0.0f <= paramV && paramU + paramV <= 1.0f)
	{
		outResult.t = t;
		outResult.p = p;
		outResult.n = normalize((1 - paramU - paramV) * n0 + paramU * n1 + paramV * n2);
		outResult.paramU = (1 - paramU - paramV) * s0 + paramU * s1 + paramV * s2;
		outResult.paramV = (1 - paramU - paramV) * t0 + paramU * t1 + paramV * t2;
		outResult.material = material;

		return true;
	}

	return false;
}

bool Triangle::BoundingBox(float t0, float t1, AABB& outBox) const
{
	outBox = bounds;
	return true;
}

void Triangle::GetVertices(vec3& outV0, vec3& outV1, vec3& outV2) const
{
	outV0 = v0;
	outV1 = v1;
	outV2 = v2;
}

void Triangle::SetVertices(const vec3& inV0, const vec3& inV1, const vec3& inV2)
{
	v0 = inV0;
	v1 = inV1;
	v2 = inV2;
	UpdateNormal();
	bounds = AABB(min(min(v0, v1), v2), max(max(v0, v1), v2));
}

void Triangle::GetNormals(vec3& outN0, vec3& outN1, vec3& outN2) const
{
	outN0 = n0;
	outN1 = n1;
	outN2 = n2;
}

void Triangle::SetNormals(const vec3& inN0, const vec3& inN1, const vec3& inN2)
{
	n0 = inN0;
	n1 = inN1;
	n2 = inN2;
}
