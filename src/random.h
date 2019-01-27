#pragma once

#include "type.h"
#include <random>
#include <vector>

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

	inline void Seek(int32 ix) { index = ix; }
	inline float Peek() const
	{
		float x = samples[index];
		index = (index + 1) % samples.size();
		return x;
	}

	std::vector<float> samples;

private:
	mutable int32 index;

};

