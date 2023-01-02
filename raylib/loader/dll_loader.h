// Run-time dynamic linking of DLLs.

#pragma once

#include "core/platform.h"

#if PLATFORM_WINDOWS
	#include <Windows.h>
#endif

// Wanna hide these third-party includes but a) not this is not a C++20 project
// and b) C++ module support is not universal afaik.
#include <FreeImage.h>

namespace FreeImage
{
	bool LoadDLL();

	using PFN_Initialise          = decltype(&FreeImage_Initialise);
	using PFN_DeInitialise        = decltype(&FreeImage_DeInitialise);
	using PFN_GetFIFFromFilename  = decltype(&FreeImage_GetFIFFromFilename);
	using PFN_Load                = decltype(&FreeImage_Load);
	using PFN_ConvertToRGBAF      = decltype(&FreeImage_ConvertToRGBAF);
	using PFN_Unload              = decltype(&FreeImage_Unload);
	using PFN_GetBits             = decltype(&FreeImage_GetBits);
	using PFN_GetWidth            = decltype(&FreeImage_GetWidth);
	using PFN_GetHeight           = decltype(&FreeImage_GetHeight);
	using PFN_GetPitch            = decltype(&FreeImage_GetPitch);
	using PFN_ConvertTo32Bits     = decltype(&FreeImage_ConvertTo32Bits);
	using PFN_Save                = decltype(&FreeImage_Save);

	extern PFN_Initialise         Initialise;
	extern PFN_DeInitialise       DeInitialise;
	extern PFN_GetFIFFromFilename GetFIFFromFilename;
	extern PFN_Load               Load;
	extern PFN_ConvertToRGBAF     ConvertToRGBAF;
	extern PFN_Unload             Unload;
	extern PFN_GetBits            GetBits;
	extern PFN_GetWidth           GetWidth;
	extern PFN_GetHeight          GetHeight;
	extern PFN_GetPitch           GetPitch;
	extern PFN_ConvertTo32Bits    ConvertTo32Bits;
	extern PFN_Save               Save;
}
