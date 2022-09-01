#pragma once

#include "core/vec.h"

class ray
{

public:
	ray() : t(0.0f) {}
	ray(const vec3& origin, const vec3& direction, float worldTime)
		: o(origin)
		, d(direction)
		, t(worldTime)
	{
	}

	vec3 at(float t) const { return o + t * d; }

	vec3 o; // origin
	vec3 d; // direction
	float t; // world time when this ray was generated, not relevant to the parameter 't' of at().

};
