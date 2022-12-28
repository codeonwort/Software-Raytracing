#pragma once

#include "raylib.h"
#include "platform.h"
#include <assert.h>

extern "C" {

	// Avoid inlining; it will mess up assembly codegen.
	RAYLIB_API void CHECK_IMPL(int x, const char* file, int line);
	RAYLIB_API void CHECKF_IMPL(int x, const char* msg, const char* file, int line);

	#define CHECK(x)         CHECK_IMPL(!!(x), __FILE__, __LINE__)
	#define CHECKF(x, msg)   CHECKF_IMPL(!!(x), msg, __FILE__, __LINE__)
	#define CHECK_NO_ENTRY() CHECK(false);

	#define STATIC_ASSERT(x) static_assert(x)

}
