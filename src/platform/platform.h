#pragma once

// Windows
#if (defined(_WIN32) && _WIN32) || (defined(_WIN64) && _WIN64)
	#define PLATFORM_WINDOWS 1
#endif

#ifdef _WIN64
	#if _WIN64
		#define PLATFORM_WINDOWS 1
	#endif
#endif

#ifndef PLATFORM_WINDOWS
	#define PLATFORM_WINDOWS 0
#endif

#if asdasd
#define asd 0
#endif
