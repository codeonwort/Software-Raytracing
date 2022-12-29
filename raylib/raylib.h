#pragma once

#include "raylib_types.h"
#include "core/stat.h"
#include "core/logger.h"
#include <stdint.h>

// Main header for raytracing library.
// All symbols are exported in C style for Foreign Function Interface.

// #todo-raylib:
// 1. Move all raytracing logic to this project.
// 2. Rename 'src' as something like 'demoapp'

// -----------------------------------------------------------------------
// Public API

extern "C" {

	// -----------------------------------------------------------------------
	// Library initialization & termination

	RAYLIB_API int32_t Raylib_Initialize();
	RAYLIB_API int32_t Raylib_Terminate();

	// -----------------------------------------------------------------------
	// Manage media files

	// Load Wavefront OBJ model.
	RAYLIB_API OBJModelHandle Raylib_LoadOBJModel(const char* objPath, const char* mtldir);

	// Unload Wavefront OBJ model.
	// @return true if successful, false otherwise.
	RAYLIB_API bool Raylib_UnloadOBJModel(OBJModelHandle objHandle);

	// Load an image from an external image file.
	// To create an image on-the-fly, use Raylib_CreateImage().
	// @return NULL if failed to load.
	RAYLIB_API ImageHandle Raylib_LoadImage(const char* filepath);

	// -----------------------------------------------------------------------
	// Construct scenes

	// Create an empty scene.
	RAYLIB_API SceneHandle Raylib_CreateScene();

	// Add OBJ model to scene.
	RAYLIB_API void Raylib_AddOBJModelToScene(SceneHandle scene, OBJModelHandle objModel);

	// Finalize scene construction and prepare for rendering.
	// A scene must be finalized before rendering.
	RAYLIB_API void Raylib_FinalizeScene(SceneHandle scene);

	// Release the memory for a scene.
	RAYLIB_API bool Raylib_DestroyScene(SceneHandle sceneHandle);

	RAYLIB_API CameraHandle Raylib_CreateCamera();
	RAYLIB_API bool Raylib_DestroyCamera(CameraHandle cameraHandle);

	// Create an image.
	RAYLIB_API ImageHandle Raylib_CreateImage(uint32_t width, uint32_t height);
	RAYLIB_API bool Raylib_DestroyImage(ImageHandle imageHandle);

	// -----------------------------------------------------------------------
	// Rendering

	// Render an image. Default is path tracing, but also can render aux images.
	// @param scene        [in] The scene to render.
	// @param camera       [in] Camera from which to look at the scene.
	// @param renderMode   [in] See ERenderMode enum.
	// @param outMainImage [out] Rendered image.
	// @return 0 if successful, -1 otherwise.
	RAYLIB_API int32_t Raylib_Render(
		SceneHandle  scene,
		CameraHandle camera,
		uint32_t     renderMode,
		ImageHandle  outMainImage);

	// Denoise a noisy path traced image using Intel OpenImageDenoise.
	// You can provide optional aux images (albedo and normal) for better quality.
	// @param inMainImage      [in] Noisy path traced image.
	// @param bMainImageHDR    [in] True if the main image has HDR values.
	// @param inAlbedoImage    [in] (optional) Albedo image.
	// @param inNormalImage    [in] (optional) World normal image.
	// @param outDenoisedImage [out] denoised image.
	// @return 0 if successful, -1 otherwise.
	RAYLIB_API int32_t Raylib_Denoise(
		ImageHandle inMainImage,
		bool bMainImageHDR,
		ImageHandle inAlbedoImage,
		ImageHandle inNormalImage,
		ImageHandle outDenoisedImage);

	// Apply tone mapping and gamma correction.
	// #todo: Add a flag bits parameter.
	RAYLIB_API void Raylib_PostProcess(ImageHandle image);

	RAYLIB_API int32_t Raylib_IsDenoiserSupported();

	// -----------------------------------------------------------------------
	// Utils

	// See ERenderMode enum.
	// @return NULL if auxMode is invlid.
	RAYLIB_API const char* Raylib_GetRenderModeString(uint32_t auxMode);

	// Save an image data as a image file to disk.
	// #todo-lib: Support HDR formats.
	// @param image    The image to write.
	// @param filepath Target filepath.
	// @param fileType See EImageFileType. (bmp, jpg, png, ...)
	// @return true if successful, false otherwise.
	RAYLIB_API bool Raylib_WriteImageToDisk(ImageHandle image, const char* filepath, uint32_t fileType);

	// Block current thread and wait for the log thread to emit all remaining logs.
	RAYLIB_API void Raylib_FlushLogThread();

} // extern "C"
