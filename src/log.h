#pragma once

#include <mutex>
#include <stdarg.h>
#include <stdio.h>

extern std::mutex log_mutex;

inline void log(const char* format, ...)
{
	log_mutex.lock();

	va_list ap;
	va_start(ap, format);
	vfprintf(stdout, format, ap);
	va_end(ap);
	puts("");

	log_mutex.unlock();
}
