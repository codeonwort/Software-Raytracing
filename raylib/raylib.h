#pragma once

#include "raylib_types.h"
#include "core/stat.h"
#include "core/logger.h"
#include <stdint.h>

// Main header for raytracing library.
// All symbols are exported in C style for Foreign Function Interface.

// #todo-raylib:
// 2. Rename 'src' as something like 'demoapp'

// -----------------------------------------------------------------------
// Public API

extern "C" {

	// -----------------------------------------------------------------------
	// Library initialization & termination

	// @return 1 if successful, 0 otherwise.
	RAYLIB_API int32_t Raylib_Initialize();

	// @return 1 if successful, 0 otherwise.
	RAYLIB_API int32_t Raylib_Terminate();

	// -----------------------------------------------------------------------
	// Manage media files

	// Load Wavefront OBJ model.
	// Should finalize via Raylib_FinalizeOBJModel() before adding to scene.
	// Add to scene via Raylib_AddOBJModelToScene().
	RAYLIB_API OBJModelHandle Raylib_LoadOBJModel(const char* objPath);

	RAYLIB_API void Raylib_TransformOBJModel(
		OBJModelHandle objModel,
		float translationX, float translationY, float translationZ,
		float yaw, float pitch, float roll,
		float scaleX, float scaleY, float scaleZ);

	RAYLIB_API void Raylib_FinalizeOBJModel(OBJModelHandle objModel);

	// Unload Wavefront OBJ model.
	// @return 1 if successful, 0 otherwise.
	RAYLIB_API int32_t Raylib_UnloadOBJModel(OBJModelHandle objHandle);

	// Load an image from an external image file.
	// To create an image on-the-fly, use Raylib_CreateImage().
	// @return NULL if failed to load.
	RAYLIB_API ImageHandle Raylib_LoadImage(const char* filepath);

	// -----------------------------------------------------------------------
	// Scene

	// Create an empty scene.
	RAYLIB_API SceneHandle Raylib_CreateScene();

	// Add scene element. (SceneElementHandle = Hitable*)
	// OBJModel should be added by Raylib_AddOBJModelToScene().
	RAYLIB_API void Raylib_AddSceneElement(SceneHandle scene, SceneElementHandle element);

	// Add OBJ model to scene.
	RAYLIB_API void Raylib_AddOBJModelToScene(SceneHandle scene, OBJModelHandle objModel);

	RAYLIB_API void Raylib_SetSkyPanorama(SceneHandle scene, ImageHandle skyImage);
	RAYLIB_API void Raylib_SetSunIlluminance(SceneHandle scene, float r, float g, float b);
	RAYLIB_API void Raylib_SetSunDirection(SceneHandle scene, float x, float y, float z);

	// Finalize scene construction and prepare for rendering.
	// A scene must be finalized before rendering.
	RAYLIB_API void Raylib_FinalizeScene(SceneHandle scene);

	// Release the memory for a scene.
	RAYLIB_API int32_t Raylib_DestroyScene(SceneHandle sceneHandle);

	RAYLIB_API CameraHandle Raylib_CreateCamera();
	RAYLIB_API void Raylib_CameraSetPosition(CameraHandle camera, float x, float y, float z);
	RAYLIB_API void Raylib_CameraSetLookAt(CameraHandle camera, float tx, float ty, float tz);
	RAYLIB_API void Raylib_CameraSetPerspective(CameraHandle camera, float fovY_degrees, float aspectWH);
	RAYLIB_API void Raylib_CameraSetLens(CameraHandle camera, float aperture, float focalDistance);
	RAYLIB_API void Raylib_CameraSetMotion(CameraHandle camera, float beginTime, float endTime);
	// Copy all camera properties from src to dst.
	RAYLIB_API void Raylib_CameraCopy(CameraHandle srcCamera, CameraHandle dstCamera);
	RAYLIB_API int32_t Raylib_DestroyCamera(CameraHandle cameraHandle);

	// Create an image.
	RAYLIB_API ImageHandle Raylib_CreateImage(uint32_t width, uint32_t height);
	RAYLIB_API int32_t Raylib_DestroyImage(ImageHandle imageHandle);

	// -----------------------------------------------------------------------
	// Rendering

	// Render `scene` viewed from `camera` with given `settings`.
	// @param settings     [in] Rendering settings. (viewport size, SPP, render mode, ...)
	// @param scene        [in] The scene to render.
	// @param camera       [in] Camera from which to look at the scene.
	// @param outMainImage [out] Rendered image.
	RAYLIB_API void Raylib_Render(
		const RendererSettings* settings,
		SceneHandle  scene,
		CameraHandle camera,
		ImageHandle  outMainImage);

	// Denoise a noisy path traced image using Intel OpenImageDenoise.
	// You can provide optional aux images (albedo and normal) for better quality.
	// @param inMainImage      [in] Noisy path traced image.
	// @param bMainImageHDR    [in] 1 if the main image has HDR values, 0 otherwise.
	// @param inAlbedoImage    [in] (optional) Albedo image.
	// @param inNormalImage    [in] (optional) World normal image.
	// @param outDenoisedImage [out] denoised image.
	// @return 1 if successful, 0 otherwise.
	RAYLIB_API int32_t Raylib_Denoise(
		ImageHandle inMainImage,
		int32_t bMainImageHDR,
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
	// @return 1 if successful, 0 otherwise.
	RAYLIB_API int32_t Raylib_WriteImageToDisk(ImageHandle image, const char* filepath, uint32_t fileType);

	// Block current thread and wait for the log thread to emit all remaining logs.
	RAYLIB_API void Raylib_FlushLogThread();

} // extern "C"
