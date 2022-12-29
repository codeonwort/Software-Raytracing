#include "raylib.h"
#include "core/platform.h"
#include "core/concurrent_vector.h"
#include "core/logger.h"
#include "render/camera.h"
#include "render/image.h"
#include "render/renderer.h"

#include <iostream>

// -----------------------------------------------------------------------

static concurrent_vector<Camera*>  g_cameras;
static concurrent_vector<Image2D*> g_images;

// -----------------------------------------------------------------------

int32_t Raylib_Initialize()
{
	std::cout << "Initialize raylib" << std::endl;

	Logger::StartLogThread();
	ImageIO::InitializeImageIO();

	return 0;
}

int32_t Raylib_Terminate()
{
	std::cout << "Terminate raylib" << std::endl;

	ImageIO::TerminateImageIO();
	Logger::KillAndWaitForLogThread();

	return 0;
}

CameraHandle Raylib_CreateCamera()
{
	Camera* camera = new Camera;
	g_cameras.push_back(camera);
	return (CameraHandle)camera;
}

bool Raylib_DestroyCamera(CameraHandle cameraHandle)
{
	Camera* camera = (Camera*)cameraHandle;
	if (g_cameras.erase_first(camera))
	{
		delete camera;
		return true;
	}
	return false;
}

ImageHandle Raylib_CreateImage(uint32_t width, uint32_t height)
{
	Image2D* image = new Image2D(width, height, 0x0);
	g_images.push_back(image);
	return (ImageHandle)image;
}

bool Raylib_DestroyImage(ImageHandle imageHandle)
{
	Image2D* image = (Image2D*)imageHandle;
	if (g_images.erase_first(image))
	{
		delete image;
		return true;
	}
	return false;
}

ImageHandle Raylib_LoadImage(const char* filepath)
{
	Image2D* image = ImageIO::LoadImage2DFromFile(filepath);
	g_images.push_back(image);
	return (ImageHandle)image;
}

// -----------------------------------------------------------------------
// Rendering

int32_t Raylib_Denoise(
	ImageHandle inMainImage,
	bool bMainImageHDR,
	ImageHandle inAlbedoImage,
	ImageHandle inNormalImage,
	ImageHandle outDenoisedImage)
{
	Renderer renderer;
	bool bRet = renderer.DenoiseScene(
		(Image2D*)inMainImage,
		bMainImageHDR,
		(Image2D*)inAlbedoImage,
		(Image2D*)inNormalImage,
		(Image2D*)outDenoisedImage);
	return (bRet ? 0 : -1);
}

void Raylib_PostProcess(ImageHandle image)
{
	((Image2D*)image)->PostProcess();
}

int32_t Raylib_IsDenoiserSupported()
{
	return Renderer::IsDenoiserSupported() ? 1 : 0;
}

// -----------------------------------------------------------------------
// Utils

RAYLIB_API const char* Raylib_GetRenderModeString(uint32_t auxMode)
{
	static const char* enumStrs[] = {
		"Default",
		"Albedo",
		"SurfaceNormal",
		"MicrosurfaceNormal",
		"Texcoord",
		"Emission",
		"Reflectance",
	};

	bool bValid = (0 <= auxMode && auxMode < RAYLIB_RENDERMODE_MAX);
	return bValid ? enumStrs[auxMode] : nullptr;
}

bool Raylib_WriteImageToDisk(ImageHandle imageHandle, const char* filepath, uint32_t fileTypeRaw)
{
	if (imageHandle == NULL || filepath == nullptr || fileTypeRaw >= EImageFileType::RAYLIB_IMAGEFILETYPE_MAX)
	{
		return false;
	}

	Image2D* image = (Image2D*)imageHandle;
	EImageFileType fileType = (EImageFileType)fileTypeRaw;

	// #todo-raylib: Activate after all images are allocated by Raylib_CreateImage().
#if 0
	if (!g_images.contains(image))
	{
		// LOG("Invalid image handle")
		return false;
	}
#endif

	return ImageIO::WriteImage2DToDisk(image, filepath, fileType);
}

void Raylib_FlushLogThread()
{
	Logger::FlushLogThread();
}
