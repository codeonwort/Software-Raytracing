#pragma once

#include "type.h"

#include <random>
#include <vector>
#include <algorithm>

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
		index = ix;
	}

	inline float Peek() const
	{
		float x = samples[index];

		// #todo: Regenerate on starvation?
		index = (index + 1) % samples.size();

		return x;
	}

private:
	std::vector<float> samples;
	std::random_device rd;

	mutable int32 index;

};

vec3 RandomInUnitSphere();

float Random();

vec3 RandomInUnitDisk();
