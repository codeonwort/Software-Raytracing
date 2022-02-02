#pragma once

#include "util/assertion.h"

#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <algorithm>

using int8   = int8_t;
using int16  = int16_t;
using int32  = int32_t;
using int64  = int64_t;

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

class String
{

public:
	String(const char* x0) { x = x0; }

	const char* GetData() const { return x; }

private:
	const char* x;

};

// #todo: May need to separate into vec3.h
class vec3
{

public:
	vec3() : vec3(0.0f, 0.0f, 0.0f) {}
	vec3(float e0, float e1, float e2)
	{
		x = e0;
		y = e1;
		z = e2;
	}

	inline const vec3& operator+() const { return *this; }
	inline vec3 operator-() const { return vec3(-x, -y, -z); }

	inline vec3& operator+=(const vec3 &v2);
	inline vec3& operator-=(const vec3 &v2);
	inline vec3& operator*=(const vec3 &v2);
	inline vec3& operator/=(const vec3 &v2);
	inline vec3& operator+=(const float t);
	inline vec3& operator-=(const float t);
	inline vec3& operator*=(const float t);
	inline vec3& operator/=(const float t);

	inline float operator[](int32 ix) const;

	inline float Length() const { return sqrtf(x*x + y*y + z*z); }
	inline float LengthSquared() const { return (x*x + y*y + z*z); }
	inline void Normalize();

	float x;
	float y;
	float z;

};

inline void vec3::Normalize()
{
	float k = 1.0f / Length();
	x *= k;
	y *= k;
	z *= k;
}

inline vec3 operator+(const vec3& v1, const vec3& v2)
{
	return vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}
inline vec3 operator-(const vec3& v1, const vec3& v2)
{
	return vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}
inline vec3 operator*(const vec3& v1, const vec3& v2)
{
	return vec3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}
inline vec3 operator/(const vec3& v1, const vec3& v2)
{
	return vec3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
}

inline vec3 operator+(const vec3& v1, float t)
{
	return vec3(v1.x + t, v1.y + t, v1.z + t);
}
inline vec3 operator+(float t, const vec3& v1)
{
	return vec3(v1.x + t, v1.y + t, v1.z + t);
}
inline vec3 operator-(const vec3& v1, float t)
{
	return vec3(v1.x - t, v1.y - t, v1.z - t);
}
inline vec3 operator-(float t, const vec3& v1)
{
	return vec3(v1.x - t, v1.y - t, v1.z - t);
}

inline vec3 operator*(const vec3& v1, float t)
{
	return vec3(v1.x * t, v1.y * t, v1.z * t);
}
inline vec3 operator/(const vec3& v1, float t)
{
	return vec3(v1.x / t, v1.y / t, v1.z / t);
}
inline vec3 operator/(float t, const vec3& v1)
{
	return vec3(t / v1.x, t / v1.y, t / v1.z);
}
inline vec3 operator*(float t, const vec3& v1)
{
	return vec3(v1.x * t, v1.y * t, v1.z * t);
}

inline vec3 normalize(const vec3& v)
{
	vec3 u = v;
	u.Normalize();
	return u;
}

inline float dot(const vec3& v1, const vec3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline vec3 cross(const vec3& v1, const vec3& v2)
{
	return vec3(
		v1.y * v2.z - v1.z * v2.y,
		-(v1.x * v2.z - v1.z * v2.x),
		v1.x * v2.y - v1.y * v2.x
	);
}

inline vec3 reflect(const vec3& v, const vec3& n)
{
	return v - 2.0f * dot(v, n) * n;
}

inline bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& outRefracted)
{
	vec3 uv = normalize(v);
	float dt = dot(uv, n);
	float D = 1.0f - ni_over_nt * ni_over_nt * (1.0f - dt * dt);
	if(D > 0.0f)
	{
		outRefracted = ni_over_nt * (uv - n * dt) - n * sqrtf(D);
		return true;
	}
	return false;
}

inline vec3 min(const vec3& v1, const vec3& v2)
{
	float x = std::min(v1.x, v2.x);
	float y = std::min(v1.y, v2.y);
	float z = std::min(v1.z, v2.z);
	return vec3(x, y, z);
}
inline vec3 max(const vec3& v1, const vec3& v2)
{
	float x = std::max(v1.x, v2.x);
	float y = std::max(v1.y, v2.y);
	float z = std::max(v1.z, v2.z);
	return vec3(x, y, z);
}

inline vec3& vec3::operator+=(const vec3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}
inline vec3& vec3::operator-=(const vec3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}
inline vec3& vec3::operator*=(const vec3& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}
inline vec3& vec3::operator/=(const vec3& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}
inline vec3& vec3::operator+=(const float t)
{
	x += t;
	y += t;
	z += t;
	return *this;
}
inline vec3& vec3::operator-=(const float t)
{
	x -= t;
	y -= t;
	z -= t;
	return *this;
}
inline vec3& vec3::operator*=(const float t)
{
	x *= t;
	y *= t;
	z *= t;
	return *this;
}
inline vec3& vec3::operator/=(const float t)
{
	float k = 1.0f / t;
	x *= k;
	y *= k;
	z *= k;
	return *this;
}

inline float vec3::operator[](int32 ix) const
{
	if (ix == 0) return x;
	else if (ix == 1) return y;
	else if (ix == 2) return z;

	CHECK_NO_ENTRY();
	return NAN;
}
