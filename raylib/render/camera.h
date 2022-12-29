#pragma once

#include "core/random.h"
#include "geom/ray.h"

#include <math.h>

template<typename T>
constexpr T pi = T(3.1415926535897932385);

class Camera
{
public:
	Camera()
	{
		Camera(
			vec3(0.0f), vec3(0.0f, 0.0f, -1.0f),
			60.0f, 16.0f / 9.0f,
			0.0f, 1.0f,
			0.0f, 0.0f);
	}

	// fov_y in degrees
	// aspect = screen_width / screen_height
	Camera(
		const vec3& inLocation, const vec3& inLookAt,
		float inFovY_degrees, float inAspectWH,
		float inAperture, float inFocalDistance,
		float inBeginTime, float inEndTime)
	{
		origin        = inLocation;
		lookAt        = inLookAt;
		fovY_degrees  = inFovY_degrees;
		aspectWH      = inAspectWH;
		aperture      = inAperture;
		focalDistance = inFocalDistance;
		beginTime     = inBeginTime;
		endTime       = inEndTime;

		UpdateInternal();
	}

	// s, t: Relative viewport coords in [0.0, 1.0)
	ray GetCameraRay(float s, float t) const
	{
		vec3 rd     = lensRadius * RandomInUnitDisk();
		vec3 offset = (u * rd.x) + (v * rd.y);
		float captureTime = beginTime + timePeriod * Random();

		vec3 rayO = origin + offset;
		vec3 rayD = normalize(top_left + s * horizontal + (1.0f - t) * vertical - origin - offset);
		return ray(rayO, rayD, captureTime);
	}

	void UpdateInternal()
	{
		lensRadius = aperture * 0.5f;
		timePeriod = endTime - beginTime;

		w = normalize(origin - lookAt);

		vec3 up(0.0f, 1.0f, 0.0f);
		if (dot(w, up) >= 0.9f)
		{
			up = vec3(1.0f, 0.0f, 0.0f);
		}

		u = normalize(cross(up, w));
		v = cross(w, u);

		float theta = fovY_degrees * pi<float> / 180.0f;
		float hh = tan(theta / 2.0f);
		float hw = aspectWH * hh;

		top_left = origin - (hw * focalDistance * u) - (hh * focalDistance * v) - (focalDistance * w);
		horizontal = 2.0f * hw * focalDistance * u;
		vertical = 2.0f * hh * focalDistance * v;
	}

// Should call UpdateInternal() whenever one of them changes.
public:
	// Position
	vec3 origin;
	vec3 lookAt;
	// Perspective
	float fovY_degrees;
	float aspectWH;
	// Lens
	float aperture;
	float focalDistance;
	// Motion
	float beginTime, endTime;

private:
	// Derived
	float lensRadius;
	float timePeriod;
	vec3 top_left;
	vec3 horizontal;
	vec3 vertical;
	vec3 u, v, w;
};
