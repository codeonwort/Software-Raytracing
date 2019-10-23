#include "transform.h"


void Transform::Init(const vec3& inLocation, const vec3& inScale)
{
	location = inLocation;
	scale = inScale;
}

void Transform::TransformVectors(const std::vector<vec3>& inVectors, std::vector<vec3>& outVectors) const
{
	int32 n = (int32)inVectors.size();
	outVectors.resize(n);

	for (int32 i = 0; i < n; ++i)
	{
		outVectors[i] = (inVectors[i] * scale) + location;
	}
}

void Transform::TransformVectors(std::vector<vec3>& vectors) const
{
	int32 n = (int32)vectors.size();
	for (int32 i = 0; i < n; ++i)
	{
		vectors[i] = (vectors[i] * scale) + location;
	}
}
