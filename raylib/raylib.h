#pragma once

#include <stdint.h>

// Main header for raytracing library.
// All symbols are exported in C style for Foreign Function Interface.

// #todo-raylib:
// 1. Move all raytracing logic to this project.
// 2. Rename 'src' as something like 'demoapp'

#ifdef RAYLIB_EXPORTS
	#define RAYLIB_API __declspec(dllexport)
#else
	#define RAYLIB_API __declspec(dllimport)
#endif

// -----------------------------------------------------------------------
// Public API

typedef uintptr_t OBJModelHandle;
typedef uintptr_t ImageHandle;
typedef uintptr_t SceneHandle;
typedef uintptr_t CameraHandle;

enum EAuxRenderMode : uint32_t
{
	Albedo             = 0,
	SurfaceNormal      = 1, // In world space.
	MicrosurfaceNormal = 2, // In local space.
	Texcoord           = 3, // Surface parameterization.
	Emission           = 4,
	Reflectance        = 5, // Can be very noisy for surfaces with diffuse materials.

	MAX
};

extern "C" RAYLIB_API const char* GetAuxRenderModeString(uint32_t auxMode);

// -----------------------------------------------------------------------
// Library initialization & termination

extern "C" RAYLIB_API int32_t Raylib_Initialize();
extern "C" RAYLIB_API int32_t Raylib_Terminate();

// -----------------------------------------------------------------------
// Manage media files

// Load Wavefront OBJ model.
extern "C" RAYLIB_API OBJModelHandle Raylib_LoadOBJModel(const char* objPath, const char* mtldir);

// Unload Wavefront OBJ model.
// @return true if successful, false otherwise.
extern "C" RAYLIB_API bool Raylib_UnloadOBJModel(OBJModelHandle objHandle);

// Load an image from an external image file.
// To create an image on-the-fly, use Raylib_CreateImage().
extern "C" RAYLIB_API ImageHandle Raylib_LoadImage(const char* imagePath);

// Create an image.
extern "C" RAYLIB_API ImageHandle Raylib_CreateImage(uint32_t width, uint32_t height);

// -----------------------------------------------------------------------
// Construct scenes

// Create an empty scene.
extern "C" RAYLIB_API SceneHandle Raylib_CreateScene();

// Add OBJ model to scene.
extern "C" RAYLIB_API void Raylib_AddOBJModelToScene(SceneHandle scene, OBJModelHandle objModel);

// Finalize scene construction and prepare for rendering.
// A scene must be finalized before rendering.
extern "C" RAYLIB_API void Raylib_FinalizeScene(SceneHandle scene);

// Release the memory for a scene.
extern "C" RAYLIB_API bool Raylib_DestroyScene(SceneHandle sceneHandle);

extern "C" RAYLIB_API CameraHandle Raylib_CreateCamera();
extern "C" RAYLIB_API bool Raylib_DestroyCamera(CameraHandle cameraHandle);

// -----------------------------------------------------------------------
// Rendering

extern "C" RAYLIB_API bool Raylib_DestroyImage(ImageHandle imageHandle);

// Generate a noisy path traced image.
// @param scene        [in] The scene to render.
// @param camera       [in] Camera from which to look at the scene.
// @param outMainImage [out] Rendered image.
// @return Returns 0 if successful, -1 otherwise.
extern "C" RAYLIB_API int32_t Raylib_Render(
	SceneHandle  scene,
	CameraHandle camera,
	ImageHandle  outMainImage);

// Render an aux image.
// @param scene       [in] The scene to render.
// @param camera      [in] Camera from which to look at the scene.
// @param auxMode     [in] See EAuxRenderMode enum.
// @param outAuxImage [out] Rendered aux image.
// @return Returns 0 if successful, -1 otherwise.
extern "C" RAYLIB_API int32_t Raylib_RenderAux(
	SceneHandle  scene,
	CameraHandle camera,
	uint32_t     auxMode,
	ImageHandle  outAuxImage);

// Denoise a noisy path traced image using Intel OpenImageDenoise.
// You can provide optional aux images (albedo and normal) for better quality.
// @param inMainImage      [in] noisy path traced image.
// @param inAlbedoImage    [in] (optional) albedo image.
// @param inNormalImage    [in] (optional) world normal image.
// @param outDenoisedImage [out] denoised image.
// @return Returns 0 if successful, -1 otherwise.
extern "C" RAYLIB_API int32_t Raylib_Denoise(
	ImageHandle inMainImage,
	ImageHandle inAlbedoImage,
	ImageHandle inNormalImage,
	ImageHandle outDenoisedImage);

// -----------------------------------------------------------------------
// Utils
