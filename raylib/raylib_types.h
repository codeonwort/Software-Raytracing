#pragma once

// Separated from raylib.h for typedefs

#include <stdint.h>

#ifdef RAYLIB_EXPORTS
	#define RAYLIB_API __declspec(dllexport)
#else
	#define RAYLIB_API __declspec(dllimport)
#endif

typedef uintptr_t OBJModelHandle;
typedef uintptr_t ImageHandle;
typedef uintptr_t SceneHandle;
typedef uintptr_t CameraHandle;

enum ERenderMode
{
	RAYLIB_RENDERMODE_Default            = 0, // Path tracing.
	RAYLIB_RENDERMODE_Albedo             = 1,
	RAYLIB_RENDERMODE_SurfaceNormal      = 2, // In world space.
	RAYLIB_RENDERMODE_MicrosurfaceNormal = 3, // In world space.
	RAYLIB_RENDERMODE_Texcoord           = 4, // Surface parameterization.
	RAYLIB_RENDERMODE_Emission           = 5,
	RAYLIB_RENDERMODE_Reflectance        = 6, // Can be very noisy for surfaces with diffuse materials.

	RAYLIB_RENDERMODE_MAX
};

enum EImageFileType
{
	RAYLIB_IMAGEFILETYPE_Bitmap = 0,
	RAYLIB_IMAGEFILETYPE_Jpg    = 1,
	RAYLIB_IMAGEFILETYPE_Png    = 2,

	RAYLIB_IMAGEFILETYPE_MAX
};
