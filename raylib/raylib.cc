#include "raylib.h"
#include "core/platform.h"
#include "core/concurrent_vector.h"
#include "render/camera.h"

#include <iostream>

#pragma warning(push)
#pragma warning(disable: 4819)
#include "FreeImage.h"
#pragma warning(pop)

// -----------------------------------------------------------------------

static concurrent_vector<Camera*> g_cameras;

// -----------------------------------------------------------------------

int32_t Raylib_Initialize()
{
	std::cout << "Initialize raylib" << std::endl;
	return 0;
}

int32_t Raylib_Terminate()
{
	std::cout << "Terminate raylib" << std::endl;
	return 0;
}

void Raylib_WriteImageToDisk(ImageHandle image, const char* filepath, uint32_t fileType)
{
	//
}

RAYLIB_API CameraHandle Raylib_CreateCamera()
{
	Camera* camera = new Camera;
	g_cameras.push_back(camera);
	return (CameraHandle)camera;
}

RAYLIB_API bool Raylib_DestroyCamera(CameraHandle cameraHandle)
{
	Camera* camera = (Camera*)cameraHandle;
	if (g_cameras.erase_first(camera))
	{
		delete camera;
		return true;
	}
	return false;
}
