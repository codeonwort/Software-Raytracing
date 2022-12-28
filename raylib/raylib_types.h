#pragma once

typedef uintptr_t OBJModelHandle;
typedef uintptr_t ImageHandle;
typedef uintptr_t SceneHandle;
typedef uintptr_t CameraHandle;

enum EAuxRenderMode
{
	RAYLIB_AUXRENDERMODE_Albedo = 0,
	RAYLIB_AUXRENDERMODE_SurfaceNormal = 1, // In world space.
	RAYLIB_AUXRENDERMODE_MicrosurfaceNormal = 2, // In local space.
	RAYLIB_AUXRENDERMODE_Texcoord = 3, // Surface parameterization.
	RAYLIB_AUXRENDERMODE_Emission = 4,
	RAYLIB_AUXRENDERMODE_Reflectance = 5, // Can be very noisy for surfaces with diffuse materials.

	RAYLIB_AUXRENDERMODE_MAX
};

enum EImageFileType
{
	RAYLIB_IMAGEFILETYPE_Bitmap = 0,
	RAYLIB_IMAGEFILETYPE_Jpg = 1,
	RAYLIB_IMAGEFILETYPE_Png = 2,

	RAYLIB_IMAGEFILETYPE_MAX
};
