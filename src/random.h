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

vec3 RandomInUnitSphere();

float Random();

vec3 RandomInUnitDisk();
