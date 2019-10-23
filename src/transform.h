#pragma once

#include "type.h"
#include <vector>

// #todo: Support rotation (needs quaternion)
// #todo: Utilize SIMD
class Transform
{
	
public:
	Transform()
	{
		Init(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));
	}

	void Init(const vec3& inLocation, const vec3& inScale);

	inline void SetLocation(const vec3& inLocation)
	{
		Init(inLocation, scale);
	}
	inline void SetScale(const vec3& inScale)
	{
		Init(location, inScale);
	}

	void TransformVectors(std::vector<vec3>& vectors) const;
	void TransformVectors(const std::vector<vec3>& inVectors, std::vector<vec3>& outVectors) const;

private:
	vec3 location;
	vec3 scale;

};
