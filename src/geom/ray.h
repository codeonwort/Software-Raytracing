#pragma once

#include "type.h"

class ray
{

public:
	ray() {}
	ray(const vec3& origin, const vec3& direction)
	{
		o = origin;
		d = direction;
	}

	vec3 at(float t) const { return o + t * d; }

	vec3 o; // origin
	vec3 d; // direction

};

