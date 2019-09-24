#pragma once

#include <assert.h>

#define CHECK(x) assert(x)
#define CHECK_NO_ENTRY() CHECK(false)

#define STATIC_ASSERT(x) static_assert(x)
