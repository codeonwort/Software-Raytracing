// Statistics about cpu cycle and memory usage.

#pragma once

#include "core/logger.h"
#include <chrono>

struct ScopedCycleCounter
{
	ScopedCycleCounter(const char* fnName)
		: label(fnName)
	{
		startTime = std::chrono::system_clock::now();
	}

	~ScopedCycleCounter()
	{
		auto diff = std::chrono::system_clock::now() - startTime;
		auto elapsedMS = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
		float seconds = (float)elapsedMS * 0.001f;

		LOG("[STAT] %s: %u ms (%.3f s)", label, elapsedMS, seconds);
	}

private:
	const char* label;
	std::chrono::system_clock::time_point startTime;

};

#define SCOPED_CPU_COUNTER(custom_label) ScopedCycleCounter __scoped_cycle_counter(#custom_label);
