#pragma once

#include "type.h"

#include <random>
#include <vector>
#include <algorithm>

// #todo: Let each thread use its own RNG.
// Those seekGuard and peekGuard cause unnecessary overhead.
#define RANDOM_LOCK_GUARD 0
class RNG
{

public:
	RNG(uint32 nSamples)
	{
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dist(0.0, 1.0);

		samples.resize(nSamples);
		for(uint32 i = 0; i < nSamples; ++i)
		{
			samples[i] = (float)dist(gen);
		}

		Seek(0);
	}

	inline void Seek(int32 ix)
	{
#if RANDOM_LOCK_GUARD
		seekGuard.lock();
#endif

		index = ix;

#if RANDOM_LOCK_GUARD
		seekGuard.unlock();
#endif
	}

	inline float Peek() const
	{
#if RANDOM_LOCK_GUARD
		peekGuard.lock();
#endif

		float x = samples[index];

#if RANDOM_LOCK_GUARD
		peekGuard.unlock();
#endif

		return x;
	}

private:
	std::vector<float> samples;
	std::random_device rd;

private:
	std::random_device rd;

	mutable int32 index;

#if RANDOM_LOCK_GUARD
	std::mutex seekGuard;
	mutable std::mutex peekGuard;
#endif

};

vec3 RandomInUnitSphere()
{
	static thread_local RNG randoms(1024);

#if 0 // original impl. of the tutorial
	vec3 p;
	do
	{
		p = 2.0f * vec3(randoms.Peek(), randoms.Peek(), randoms.Peek()) - vec3(1.0f, 1.0f, 1.0f);
	}
	while(p.LengthSquared() >= 1.0f);
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

float Random()
{
	static thread_local RNG randoms(1024 * 8);

	return randoms.Peek();
}

vec3 RandomInUnitDisk()
{
	static thread_local RNG randoms(1024 * 8);
	float u1 = randoms.Peek();
	float u2 = randoms.Peek();
	float r = sqrt(u1);
	float theta = 2.0f * (float)M_PI * u2;
	return vec3(r * cos(theta), r * sin(theta), 0.0f);
}
