#pragma once

#include "random.h"
#include "geom/ray.h"

#include <math.h>

template<typename T>
constexpr T pi = T(3.1415926535897932385);

class Camera
{

public:
	// fov_y in degrees
	// aspect = screen_width / screen_height
	Camera(
		const vec3& location, const vec3& lookAt, const vec3& up,
		float fov_y, float aspect,
		float aperture, float focus_dist,
		float timeBeginCapture, float timeEndCapture)
	{
		lens_radius       = aperture * 0.5f;
		float theta       = fov_y * pi<float> / 180.0f;
		float half_height = tan(theta / 2.0f);
		float half_width  = aspect * half_height;

		origin = location;
		w = normalize(location - lookAt);
		u = normalize(cross(up, w));
		v = cross(w, u);

		//top_left = vec3(-half_width, half_height, -1.0f);
		top_left   = origin - (half_width * focus_dist * u) - (half_height * focus_dist * v) - (focus_dist * w);
		horizontal = 2.0f * half_width * focus_dist * u;
		vertical   = 2.0f * half_height * focus_dist * v;
		//horizontal = vec3(2.0f * half_width, 0.0f, 0.0f);
		//vertical = vec3(0.0f, -2.0f * half_height, 0.0f);

		beginCapture = timeBeginCapture;
		endCapture = timeEndCapture;
	}

	ray GetRay(float s, float t)
	{
		vec3 rd     = lens_radius * RandomInUnitDisk();
		vec3 offset = (u * rd.x) + (v * rd.y);
		float captureTime = beginCapture + (endCapture - beginCapture) * Random();
		return ray(origin + offset, top_left + s * horizontal + (1.0f - t) * vertical - origin - offset, captureTime);
	}

	vec3 origin;
	vec3 top_left;
	vec3 horizontal;
	vec3 vertical;
	vec3 u, v, w;
	float beginCapture, endCapture;
	float lens_radius;

};
