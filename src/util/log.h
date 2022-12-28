#pragma once

// Thread-safe async logging util.
// CAUTION: Won't synchronize with printf and std::cout.

// Start a logging thread.
void StartLogThread();

// Block current thread and wait for all logs to be printed.
void FlushLogThread();

// Print remaining log and kill the logging thread.
void WaitForLogThread();

// Queue a log message. Format rules are same as printf.
void LOG(const char* format, ...);
