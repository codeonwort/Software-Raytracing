#include "image_loader.h"
#include "log.h"
#include "util/resource_finder.h"
#include "util/assertion.h"

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

bool ImageLoader::SyncLoad(const char* filepath, Image2D& outImage)
{
	ImageLoader loader;
	return loader.LoadSynchronous(filepath, outImage);
}

ImageLoader::ImageLoader()
{
}

ImageLoader::~ImageLoader()
{
	// #todo-imageloader: cancel async load
}

bool ImageLoader::LoadSynchronous(const char* filepath, Image2D& outImage)
{
	std::string file = ResourceFinder::Get().Find(filepath);

	if (file.size() == 0)
	{
		return false;
	}
	
	FREE_IMAGE_FORMAT FIF = FreeImage_GetFIFFromFilename(file.data());
	FIBITMAP* dib = FreeImage_Load(FIF, file.data(), 0);
	CHECK(dib);
	FIBITMAP* dib32 = FreeImage_ConvertTo32Bits(dib);
	FreeImage_Unload(dib);

	// Row-major
	BYTE* colorBuffer = FreeImage_GetBits(dib32);
	uint32 width = (uint32)FreeImage_GetWidth(dib32);
	uint32 height = (uint32)FreeImage_GetHeight(dib32);
	uint32 pitch = (uint32)FreeImage_GetPitch(dib32);

	outImage.Reallocate(width, height);

	uint32 p = 0;
	for (int32 y = (int32)height - 1; y >= 0; --y)
	{
		for (int32 x = 0; x < (int32)width; ++x)
		{
			uint8 b = colorBuffer[p++];
			uint8 g = colorBuffer[p++];
			uint8 r = colorBuffer[p++];
			uint8 a = colorBuffer[p++];
			outImage.SetPixel(x, y, Pixel(r, g, b, a));
		}
	}

	FreeImage_Unload(dib32);

	return true;
}
