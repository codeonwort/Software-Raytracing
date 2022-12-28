#include "image_loader.h"
#include "core/assertion.h"
#include "util/log.h"

#pragma warning(push)
#pragma warning(disable: 4819)
#include "FreeImage.h"
#pragma warning(pop)

void ImageLoader::Initialize()
{
	FreeImage_Initialise();
	LOG("Initialize image loader");
}

void ImageLoader::Destroy()
{
	FreeImage_DeInitialise();
	LOG("Destroy image loader");
}

bool ImageLoader::SyncLoad(const char* filepath, Image2D& outImage)
{
	ImageLoader loader;
	return loader.LoadSynchronous(filepath, outImage);
}

bool ImageLoader::SyncLoad(const char* filepath, std::shared_ptr<Image2D>& outImage)
{
	outImage = std::make_shared<Image2D>();
	if (SyncLoad(filepath, *outImage.get()))
	{
		return true;
	}
	outImage = nullptr;
	return false;
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
	if (filepath == nullptr)
	{
		return false;
	}
	
	FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFIFFromFilename(filepath);
	FIBITMAP* dib = FreeImage_Load(imageFormat, filepath, 0);

	if (dib == nullptr)
	{
		return false;
	}

	if (imageFormat == FIF_HDR)
	{
		FIBITMAP* dibF = FreeImage_ConvertToRGBAF(dib);
		FreeImage_Unload(dib);

		float* hdrBuffer = reinterpret_cast<float*>(FreeImage_GetBits(dibF));
		uint32 width = (uint32)FreeImage_GetWidth(dibF);
		uint32 height = (uint32)FreeImage_GetHeight(dibF);
		uint32 pitch = (uint32)FreeImage_GetPitch(dibF);

		outImage.Reallocate(width, height);

		uint32 p = 0;
		for (int32 y = (int32)height - 1; y >= 0; --y)
		{
			for (int32 x = 0; x < (int32)width; ++x)
			{
				float r = hdrBuffer[p++];
				float g = hdrBuffer[p++];
				float b = hdrBuffer[p++];
				float a = hdrBuffer[p++];
				outImage.SetPixel(x, y, Pixel(r, g, b, a));
			}
		}

		FreeImage_Unload(dibF);
	}
	else
	{
		if (imageFormat != FIF_BMP && imageFormat != FIF_JPEG && imageFormat != FIF_PNG)
		{
			LOG("%s: unexpected image format. Will be interpreted as rgba8.");
		}

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
	}

	return true;
}
