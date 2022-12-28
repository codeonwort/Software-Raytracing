#include "raylib.h"
#include "core/platform.h"
#include <iostream>

#pragma warning(push)
#pragma warning(disable: 4819)
#include "FreeImage.h"
#pragma warning(pop)

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
