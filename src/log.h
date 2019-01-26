#pragma once

#include <stdarg.h>
#include <stdio.h>

inline void log(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stdout, format, ap);
	va_end(ap);
	puts("");
}

