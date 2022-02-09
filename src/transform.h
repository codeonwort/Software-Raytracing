#pragma once

#include "type.h"
#include <vector>

struct Rotator
{
	static Rotator directionToYawPitch(const vec3& dir);
	vec3 toDirection() const;
	vec3 rotate(const vec3& position) const;

	Rotator()
		: yaw(0.0f)
		, pitch(0.0f)
		, roll(0.0f)
	{}

	Rotator(float inYaw, float inPitch, float inRoll)
		: yaw(inYaw)
		, pitch(inPitch)
		, roll(inRoll)
	{}

	float yaw;   // [-180, 180]
	float pitch; // [-90, 90]
	float roll;  // [-180, 180]
};

// #todo: Generate matrix internally
// #todo: Support rotation (needs quaternion)
// #todo: Utilize SIMD
class Transform
{
	
public:
	Transform()
	{
		Init(vec3(0.0f, 0.0f, 0.0f), Rotator(), vec3(1.0f, 1.0f, 1.0f));
	}

	void Init(const vec3& inLocation, const Rotator& inRotation, const vec3& inScale);

	inline void SetLocation(const vec3& inLocation)
	{
		Init(inLocation, rotation, scale);
	}
	inline void SetScale(const vec3& inScale)
	{
		Init(location, rotation, inScale);
	}

	void TransformVectors(std::vector<vec3>& vectors) const;
	void TransformVectors(const std::vector<vec3>& inVectors, std::vector<vec3>& outVectors) const;

private:
	vec3 location;
	Rotator rotation;
	vec3 scale;

};
