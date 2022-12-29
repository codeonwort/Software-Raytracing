#pragma once

#include "raylib_types.h"

// Thread-safe async logging util.
// CAUTION: Won't synchronize with printf and std::cout.

namespace Logger
{

	// Start a logging thread.
	void StartLogThread();

	// Block current thread and wait for all logs to be printed.
	RAYLIB_API void FlushLogThread();

	// Print remaining log and kill the logging thread.
	void KillAndWaitForLogThread();

}

// Queue a log message. Format rules are same as printf.
RAYLIB_API void LOG(const char* format, ...);
