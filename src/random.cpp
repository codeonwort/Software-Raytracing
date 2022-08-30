#include "random.h"

vec3 RandomInUnitSphere()
{
	static thread_local RNG randoms(4096);

#if 0 // original impl. of the tutorial
	vec3 p;
	do
	{
		p = 2.0f * vec3(randoms.Peek(), randoms.Peek(), randoms.Peek()) - vec3(1.0f, 1.0f, 1.0f);
	} while (p.LengthSquared() >= 1.0f);
	return p;
#endif

	// PBR Ch. 13
	float u1 = randoms.Peek();
	float u2 = randoms.Peek();
	float z = 1.0f - 2.0f * u1;
	float r = sqrt(std::max(0.0f, 1.0f - z * z));
	float phi = 2.0f * 3.141592f * u2;
	return vec3(r * cos(phi), r * sin(phi), z);
}

vec3 RandomInHemisphere(const vec3& axis)
{
	vec3 v = RandomInUnitSphere();
	if (dot(v, axis) < 0.0)
	{
		v = -v;
	}
	return v;
}

float Random()
{
	static thread_local RNG randoms(4096 * 8);

	return randoms.Peek();
}

vec3 RandomInUnitDisk()
{
	static thread_local RNG randoms(4096 * 8);
	float u1 = randoms.Peek();
	float u2 = randoms.Peek();
	float r = sqrt(u1);
	float theta = 2.0f * (float)M_PI * u2;
	return vec3(r * cos(theta), r * sin(theta), 0.0f);
}

vec3 RandomInCosineHemisphere() {
	float u1 = 2.0f * Random() - 1.0f;
	float u2 = 2.0f * Random() - 1.0f;
	// Concentric disk samples
	{
		float theta, r;
		if (u1 != 0.0f && u2 != 0.0f) {
			if (std::abs(u1) > std::abs(u2)) {
				r = u1;
				theta = M_PI_4 * (u2 / u1);
			} else {
				r = u2;
				theta = M_PI_2 - M_PI_4 * (u1 / u2);
			}
			u1 = r * std::cos(theta);
			u2 = r * std::sin(theta);
		}
	}

	float z = std::sqrt(std::max(0.0f, 1.0f - u1 * u1 - u2 * u2));
	return vec3(u1, u2, z);
}
