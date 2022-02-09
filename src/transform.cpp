#include "transform.h"

// #todo: Common math library
template<typename T>
constexpr static T pi = T(3.1415926535897932385);

static float toDegrees(float radians)
{
	return radians * 180.0f / pi<float>;
}
static float toRadians(float degrees)
{
	return degrees * pi<float> / 180.0f;
}

/////////////////////////////////////////////////////////////////
// Rotator

Rotator Rotator::directionToYawPitch(const vec3& dir)
{
	const float mag = dir.Length();
	if (mag < 0.0001f)
	{
		return Rotator(0.0f, 0.0f, 0.0f);
	}

	// https://gamedev.stackexchange.com/questions/172147/convert-3d-direction-vectors-to-yaw-pitch-roll-angles
	const vec3 up(0.0f, 1.0f, 0.0f);
	const vec3 forward(1.0f, 0.0f, 0.0f);
	float yaw = atan2(-dir.z, dir.x);
	float pitch = asinf(dir.y / mag);
	float planeRightX = sin(yaw);
	float planeRightY = -cos(yaw);
	float roll = asin(up.x * planeRightX + up.z * planeRightY);
	if (up.y < 0.0f) roll = (roll < 0.0f ? -1.0f : 1.0f) * pi<float> -roll;
	return Rotator(toDegrees(yaw), toDegrees(pitch), toDegrees(roll));
}

vec3 Rotator::toDirection() const
{
	float theta = toRadians(yaw);
	float phi = toRadians(pitch);
	float cosPhi = ::cosf(phi);
	return vec3(sinf(theta) * cosPhi, sinf(phi), cosf(theta) * cosPhi);
}

vec3 Rotator::rotate(const vec3& position) const
{
	const float rad_yaw = toRadians(yaw);
	const float rad_pitch = toRadians(pitch);
	const float rad_roll = toRadians(roll);
	const float ch = ::cosf(rad_yaw);
	const float sh = ::sinf(rad_yaw);
	const float cp = ::cosf(rad_pitch);
	const float sp = ::sinf(rad_pitch);
	const float cb = ::cosf(rad_roll);
	const float sb = ::sinf(rad_roll);

	vec3 M[3] = {
		vec3{ch * cb + sh * sp * sb, sb * cp, -sh * cb + ch * sp * sb},
		vec3{-ch * sb + sh * sp * cb, cb * cp, sb * sh + ch * sp * cb},
		vec3{sh * cp, -sp, ch * cp}
	};
	return vec3(dot(M[0], position), dot(M[1], position), dot(M[2], position));
}

/////////////////////////////////////////////////////////////////
// Transform

void Transform::Init(const vec3& inLocation, const Rotator& inRotation, const vec3& inScale)
{
	location = inLocation;
	rotation = inRotation;
	scale = inScale;
}

void Transform::TransformVectors(const std::vector<vec3>& inVectors, std::vector<vec3>& outVectors) const
{
	int32 n = (int32)inVectors.size();
	outVectors.resize(n);

	for (int32 i = 0; i < n; ++i)
	{
		outVectors[i] = (rotation.rotate(inVectors[i]) * scale) + location;
	}
}

void Transform::TransformVectors(std::vector<vec3>& vectors) const
{
	int32 n = (int32)vectors.size();
	for (int32 i = 0; i < n; ++i)
	{
		vectors[i] = (rotation.rotate(vectors[i]) * scale) + location;
	}
}
