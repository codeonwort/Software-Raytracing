#pragma once

#include "ray.h"

class Camera
{

public:
	Camera()
		: top_left(vec3(-2.0f, 1.0f, -1.0f))
		, horizontal(vec3(4.0f, 0.0f, 0.0f))
		, vertical(vec3(0.0f, -2.0f, 0.0f))
		, origin(vec3(0.0f, 0.0f, 0.0f))
	{
	}

	ray GetRay(float u, float v)
	{
		return ray(origin, top_left + u * horizontal + v * vertical - origin);
	}

	vec3 origin;
	vec3 top_left;
	vec3 horizontal;
	vec3 vertical;

};

