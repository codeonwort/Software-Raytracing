#pragma once

#include <stdint.h>

using int8   = int8_t;
using int16  = int16_t;
using int32  = int32_t;
using int64  = int64_t;

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

class String
{

public:
	String(const char* x0) { x = x0; }

	const char* GetData() const { return x; }

private:
	const char* x;

};

