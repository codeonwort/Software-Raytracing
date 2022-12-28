#pragma once

#include "core/int_types.h"
#include "core/assertion.h"
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

struct vec3
{
	float x;
	float y;
	float z;

	vec3() : vec3(0.0f, 0.0f, 0.0f) {}
	vec3(float e0)
	{
		x = e0;
		y = e0;
		z = e0;
	}
	vec3(float e0, float e1, float e2)
	{
		x = e0;
		y = e1;
		z = e2;
	}

	inline const vec3& operator+() const { return *this; }
	inline vec3 operator-() const { return vec3(-x, -y, -z); }

	inline vec3& operator+=(const vec3& v2);
	inline vec3& operator-=(const vec3& v2);
	inline vec3& operator*=(const vec3& v2);
	inline vec3& operator/=(const vec3& v2);
	inline vec3& operator+=(const float t);
	inline vec3& operator-=(const float t);
	inline vec3& operator*=(const float t);
	inline vec3& operator/=(const float t);

	inline float operator[](int32 ix) const;

	inline float Length() const { return sqrtf(x * x + y * y + z * z); }
	inline float LengthSquared() const { return (x * x + y * y + z * z); }
	inline void Normalize();
};

inline void vec3::Normalize() {
	float k = 1.0f / Length();
	x *= k;
	y *= k;
	z *= k;
}

inline bool operator==(const vec3& v1, const vec3& v2) {
	return (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z);
}

inline vec3 operator+(const vec3& v1, const vec3& v2) {
	return vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}
inline vec3 operator-(const vec3& v1, const vec3& v2) {
	return vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}
inline vec3 operator*(const vec3& v1, const vec3& v2) {
	return vec3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}
inline vec3 operator/(const vec3& v1, const vec3& v2) {
	return vec3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
}

inline vec3 operator+(const vec3& v1, float t) {
	return vec3(v1.x + t, v1.y + t, v1.z + t);
}
inline vec3 operator+(float t, const vec3& v1) {
	return vec3(v1.x + t, v1.y + t, v1.z + t);
}
inline vec3 operator-(const vec3& v1, float t) {
	return vec3(v1.x - t, v1.y - t, v1.z - t);
}
inline vec3 operator-(float t, const vec3& v1) {
	return vec3(t - v1.x, t - v1.y, t - v1.z);
}

inline vec3 operator*(const vec3& v1, float t) {
	return vec3(v1.x * t, v1.y * t, v1.z * t);
}
inline vec3 operator/(const vec3& v1, float t) {
	return vec3(v1.x / t, v1.y / t, v1.z / t);
}
inline vec3 operator/(float t, const vec3& v1) {
	return vec3(t / v1.x, t / v1.y, t / v1.z);
}
inline vec3 operator*(float t, const vec3& v1) {
	return vec3(v1.x * t, v1.y * t, v1.z * t);
}

inline vec3 normalize(const vec3& v) {
	vec3 u = v;
	u.Normalize();
	return u;
}

inline vec3 mix(const vec3& v1, const vec3& v2, float a) {
	return (1.0f - a) * v1 + a * v2;
}

inline float dot(const vec3& v1, const vec3& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
inline float absDot(const vec3& v1, const vec3& v2) {
	return std::abs(v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

inline vec3 cross(const vec3& v1, const vec3& v2) {
	return vec3(
		v1.y * v2.z - v1.z * v2.y,
		-(v1.x * v2.z - v1.z * v2.x),
		v1.x * v2.y - v1.y * v2.x
	);
}

inline vec3 reflect(const vec3& v, const vec3& n) {
	return v - 2.0f * dot(v, n) * n;
}

inline bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& outRefracted) {
	vec3 uv = normalize(v);
	float dt = dot(uv, n);
	float D = 1.0f - ni_over_nt * ni_over_nt * (1.0f - dt * dt);
	if (D > 0.0f) {
		outRefracted = ni_over_nt * (uv - n * dt) - n * sqrtf(D);
		return true;
	}
	return false;
}

inline vec3 abs(const vec3& v) {
	return vec3(abs(v.x), abs(v.y), abs(v.z));
}

inline vec3 min(const vec3& v1, const vec3& v2) {
	float x = std::min(v1.x, v2.x);
	float y = std::min(v1.y, v2.y);
	float z = std::min(v1.z, v2.z);
	return vec3(x, y, z);
}
inline vec3 max(const vec3& v1, const vec3& v2) {
	float x = std::max(v1.x, v2.x);
	float y = std::max(v1.y, v2.y);
	float z = std::max(v1.z, v2.z);
	return vec3(x, y, z);
}
inline vec3 pow(const vec3& v, float p) {
	float x = powf(v.x, p);
	float y = powf(v.y, p);
	float z = powf(v.z, p);
	return vec3(x, y, z);
}
inline vec3 saturate(const vec3& v) {
	return max(vec3(0.0f), min(1.0f, v));
}

inline bool isnan(const vec3& v) {
	return isnan(v.x) || isnan(v.y) || isnan(v.z);
}

inline vec3& vec3::operator+=(const vec3& v) {
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}
inline vec3& vec3::operator-=(const vec3& v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}
inline vec3& vec3::operator*=(const vec3& v) {
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}
inline vec3& vec3::operator/=(const vec3& v) {
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}
inline vec3& vec3::operator+=(const float t) {
	x += t;
	y += t;
	z += t;
	return *this;
}
inline vec3& vec3::operator-=(const float t) {
	x -= t;
	y -= t;
	z -= t;
	return *this;
}
inline vec3& vec3::operator*=(const float t) {
	x *= t;
	y *= t;
	z *= t;
	return *this;
}
inline vec3& vec3::operator/=(const float t) {
	float k = 1.0f / t;
	x *= k;
	y *= k;
	z *= k;
	return *this;
}

inline float vec3::operator[](int32 ix) const {
	if (ix == 0) return x;
	else if (ix == 1) return y;
	else if (ix == 2) return z;

	CHECK_NO_ENTRY();
	return NAN;
}
