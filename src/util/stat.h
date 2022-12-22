// Statistics about cpu cycle and memory usage

#include "util/log.h"

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
		std::chrono::duration<double> diff = std::chrono::system_clock::now() - startTime;
		double seconds = diff.count();

		LOG("[STAT] %s: %lf seconds", label, seconds);
	}

private:
	const char* label;
	std::chrono::system_clock::time_point startTime;

};

#define SCOPED_CPU_COUNTER(custom_label) ScopedCycleCounter __scoped_cycle_counter(#custom_label);
