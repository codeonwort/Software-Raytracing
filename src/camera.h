#pragma once

#include "ray.h"
#include <math.h>
#include "log.h"

template<typename T>
constexpr T pi = T(3.1415926535897932385);

class Camera
{

public:
	// fov_y in degrees
	// aspect = screen_width / screen_height
	Camera(const vec3& location, const vec3& lookAt, const vec3& up, float fov_y, float aspect)
	{
		vec3 u, v, w;
		float theta = fov_y * pi<float> / 180.0f;
		float half_height = tan(theta / 2.0f);
		float half_width = aspect * half_height;

		origin = location;
		w = normalize(location - lookAt);
		u = normalize(cross(up, w));
		v = cross(w, u);

		//top_left = vec3(-half_width, half_height, -1.0f);
		top_left = origin - half_width * u - half_height * v - w;
		horizontal = 2.0f * half_width * u;
		vertical = 2.0f * half_height * v;
		//horizontal = vec3(2.0f * half_width, 0.0f, 0.0f);
		//vertical = vec3(0.0f, -2.0f * half_height, 0.0f);
	}

	ray GetRay(float u, float v)
	{
		return ray(origin, top_left + u * horizontal + (1.0f - v) * vertical - origin);
	}

	vec3 origin;
	vec3 top_left;
	vec3 horizontal;
	vec3 vertical;

};

