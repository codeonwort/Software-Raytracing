#include "raylib.h"
#include "core/platform.h"
#include "core/concurrent_vector.h"
#include "render/camera.h"
#include "render/image.h"

#include <iostream>

#pragma warning(push)
#pragma warning(disable: 4819)
#include "FreeImage.h"
#pragma warning(pop)

// -----------------------------------------------------------------------

static concurrent_vector<Camera*>  g_cameras;
static concurrent_vector<Image2D*> g_images;

// -----------------------------------------------------------------------

int32_t Raylib_Initialize()
{
	std::cout << "Initialize raylib" << std::endl;

	FreeImage_Initialise();

	return 0;
}

int32_t Raylib_Terminate()
{
	std::cout << "Terminate raylib" << std::endl;

	FreeImage_DeInitialise();

	return 0;
}

static FREE_IMAGE_FORMAT ToFreeImageType(EImageFileType inType)
{
	switch (inType)
	{
		case RAYLIB_IMAGEFILETYPE_Bitmap: return FIF_BMP;
		case RAYLIB_IMAGEFILETYPE_Jpg:    return FIF_JPEG;
		case RAYLIB_IMAGEFILETYPE_Png:    return FIF_PNG;
	}
	CHECK(false);
	return FIF_UNKNOWN;
}
void Raylib_WriteImageToDisk(ImageHandle imageHandle, const char* filepath, uint32_t fileTypeRaw)
{
	Image2D* image = (Image2D*)imageHandle;
	EImageFileType fileType = (EImageFileType)fileTypeRaw;

	// #todo-raylib: Activate after all images are allocated by Raylib_CreateImage().
#if 0
	if (!g_images.contains(image))
	{
		// LOG("Invalid image handle")
		return;
	}
#endif

	std::vector<BYTE> imageBlob;
	imageBlob.reserve(3 * image->GetWidth() * image->GetHeight());
	for (int32 y = (int32)image->GetHeight() - 1; y >= 0; --y)
	{
		for (uint32 x = 0; x < image->GetWidth(); ++x)
		{
			uint32 rgba = image->GetPixel(x, y).ToUint32();
			imageBlob.push_back(rgba & 0xff);
			imageBlob.push_back((rgba >> 8) & 0xff);
			imageBlob.push_back((rgba >> 16) & 0xff);
		}
	}
	FREE_IMAGE_FORMAT fif = ToFreeImageType(fileType);
	FIBITMAP* dib = FreeImage_ConvertFromRawBits(
		imageBlob.data(),
		image->GetWidth(), image->GetHeight(),
		image->GetWidth() * 3, 3 * 8,
		0, 0, 0);
	FreeImage_Save(fif, dib, filepath, 0);
	FreeImage_Unload(dib);
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
