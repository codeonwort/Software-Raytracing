#include "dll_loader.h"

namespace FreeImage
{

// Function pointers
PFN_Initialise         Initialise         = NULL;
PFN_DeInitialise       DeInitialise       = NULL;
PFN_GetFIFFromFilename GetFIFFromFilename = NULL;
PFN_Load               Load               = NULL;
PFN_ConvertToRGBAF     ConvertToRGBAF     = NULL;
PFN_Unload             Unload             = NULL;
PFN_GetBits            GetBits            = NULL;
PFN_GetWidth           GetWidth           = NULL;
PFN_GetHeight          GetHeight          = NULL;
PFN_GetPitch           GetPitch           = NULL;
PFN_ConvertTo32Bits    ConvertTo32Bits    = NULL;
PFN_Save               Save               = NULL;

bool LoadDLL()
{
#if PLATFORM_WINDOWS
	HINSTANCE dll = ::LoadLibrary("FreeImage.dll");
	if (dll == NULL)
	{
		return false;
	}
#define BAIL_IF_FALSE(Var, Type, ProcName)       \
	Var = (Type)::GetProcAddress(dll, ProcName); \
	if (Var == NULL) {                           \
		::FreeLibrary(dll); dll = NULL;          \
		return false;                            \
	}
	BAIL_IF_FALSE(FreeImage::Initialise,         PFN_Initialise,         "FreeImage_Initialise");
	BAIL_IF_FALSE(FreeImage::DeInitialise,       PFN_DeInitialise,       "FreeImage_DeInitialise");
	BAIL_IF_FALSE(FreeImage::GetFIFFromFilename, PFN_GetFIFFromFilename, "FreeImage_GetFIFFromFilename");
	BAIL_IF_FALSE(FreeImage::Load,               PFN_Load,               "FreeImage_Load");
	BAIL_IF_FALSE(FreeImage::ConvertToRGBAF,     PFN_ConvertToRGBAF,     "FreeImage_ConvertToRGBAF");
	BAIL_IF_FALSE(FreeImage::Unload,             PFN_Unload,             "FreeImage_Unload");
	BAIL_IF_FALSE(FreeImage::GetBits,            PFN_GetBits,            "FreeImage_GetBits");
	BAIL_IF_FALSE(FreeImage::GetWidth,           PFN_GetWidth,           "FreeImage_GetWidth");
	BAIL_IF_FALSE(FreeImage::GetHeight,          PFN_GetHeight,          "FreeImage_GetHeight");
	BAIL_IF_FALSE(FreeImage::GetPitch,           PFN_GetPitch,           "FreeImage_GetPitch");
	BAIL_IF_FALSE(FreeImage::ConvertTo32Bits,    PFN_ConvertTo32Bits,    "FreeImage_ConvertTo32Bits");
	BAIL_IF_FALSE(FreeImage::Save,               PFN_Save,               "FreeImage_Save");
#undef BAIL_IF_FALSE
#else
	#error Unsupported platform
#endif

	return true;
}

} // namespace FreeImage
