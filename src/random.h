#pragma once

#include "type.h"
#include <random>
#include <vector>
#include <algorithm>
#include <mutex>

// #todo: Let each thread use its own RNG.
// Those seekGuard and peekGuard cause unnecessary overhead.
class RNG
{

public:
	RNG(uint32 nSamples)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dist(0.0, 1.0);

		samples.resize(nSamples);
		for(uint32 i = 0; i < nSamples; ++i)
		{
			samples[i] = dist(gen);
		}

		Seek(0);
	}

	inline void Seek(int32 ix)
	{
		seekGuard.lock();

		index = ix;

		seekGuard.unlock();
	}

	inline float Peek() const
	{
		peekGuard.lock();

		float x = samples[index];
		index = (index + 1) % samples.size();

		peekGuard.unlock();

		return x;
	}

	std::vector<float> samples;

private:
	mutable int32 index;
	std::mutex seekGuard;
	mutable std::mutex peekGuard;

};

vec3 RandomInUnitSphere()
{
	static RNG randoms(1024);

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
	static RNG randoms(1024 * 8);

	return randoms.Peek();
}

vec3 RandomInUnitDisk()
{
	static RNG randoms(1024 * 8);
	float u1 = randoms.Peek();
	float u2 = randoms.Peek();
	float r = sqrt(u1);
	float theta = 2.0f * M_PI * u2;
	return vec3(r * cos(theta), r * sin(theta), 0.0f);
}

