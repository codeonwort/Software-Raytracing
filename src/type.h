#pragma once

#include <stdint.h>
#include <math.h>

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

	inline float Length() const { return sqrt(x*x + y*y + z*z); }
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
inline vec3 operator*(float t, const vec3& v1)
{
	return vec3(v1.x * t, v1.y * t, v1.z * t);
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


