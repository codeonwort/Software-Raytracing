// Very thin scene abstraction for renderer.

#pragma once

#include "raylib_types.h"
#include "hit.h"
#include "bvh.h"

class Scene
{
public:
	Scene();
	~Scene();

	void AddSceneElement(Hitable* hitable);

	// Equirectangular map
	inline void SetSkyPanorama(ImageHandle skyImage) { skyPanorama = skyImage; }
	inline void SetSunIlluminance(const vec3& illuminance) { sunIlluminance = illuminance; }
	inline void SetSunDirection(const vec3& direction) { sunDirection = normalize(direction); }

	// Construct accel struct.
	BVHNode* Finalize();

	inline ImageHandle GetSkyPanorama() const { return skyPanorama; }
	inline void GetSun(vec3& outIlluminance, vec3& outDirection) const
	{
		outIlluminance = sunIlluminance;
		outDirection = sunDirection;
	}

	inline const BVHNode* GetAccelStruct() const { return accelStruct; }

private:
	HitableList hitableList;
	BVHNode* accelStruct = nullptr;

	// Distant lighting
	ImageHandle skyPanorama = NULL;
	vec3 sunIlluminance;
	vec3 sunDirection;

	bool bFinalized = false;
};
