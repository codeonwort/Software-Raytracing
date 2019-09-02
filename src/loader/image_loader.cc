#include "image_loader.h"
#include "../log.h"

#pragma comment(lib, "FreeImage.lib")


void ImageLoader::Initialize()
{
	FreeImage_Initialise();
	log("Initialize image loader");
}

void ImageLoader::Destroy()
{
	FreeImage_DeInitialise();
	log("Destroy image loader");
}

void ImageLoader::SyncLoad(const char* filepath, HDRImage& outImage)
{
	ImageLoader loader;
	loader.LoadSynchronous(filepath, outImage);
}

ImageLoader::ImageLoader()
{
}

ImageLoader::~ImageLoader()
{
	// #todo: cancel async load
}

void ImageLoader::LoadSynchronous(const char* filepath, HDRImage& outImage)
{
	//
}
