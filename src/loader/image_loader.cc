#include "image_loader.h"
#include "util/resource_finder.h"
#include "util/assertion.h"
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
	std::string file = ResourceFinder::Get().Find(filepath);

	if (file.size() == 0)
	{
		return false;
	}
	
	FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFIFFromFilename(file.data());
	FIBITMAP* dib = FreeImage_Load(imageFormat, file.data(), 0);

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

FREE_IMAGE_FORMAT ToFreeImageType(EImageFileType inType)
{
	switch (inType)
	{
		case EImageFileType::Bitmap: return FIF_BMP;
		case EImageFileType::Jpg:    return FIF_JPEG;
		case EImageFileType::Png:    return FIF_PNG;
	}
	CHECK(false);
	return FIF_UNKNOWN;
}

void WriteImageToDisk(const Image2D& image, const char* filepath, EImageFileType fileType)
{
	std::vector<BYTE> imageBlob;
	imageBlob.reserve(3 * image.GetWidth() * image.GetHeight());
	for (int32 y = (int32)image.GetHeight() - 1; y >= 0; --y)
	{
		for (uint32 x = 0; x < image.GetWidth(); ++x)
		{
			uint32 rgba = image.GetPixel(x, y).ToUint32();
			imageBlob.push_back(rgba & 0xff);
			imageBlob.push_back((rgba >> 8) & 0xff);
			imageBlob.push_back((rgba >> 16) & 0xff);
		}
	}
	FREE_IMAGE_FORMAT fif = ToFreeImageType(fileType);
	FIBITMAP* dib = FreeImage_ConvertFromRawBits(
		imageBlob.data(),
		image.GetWidth(), image.GetHeight(),
		image.GetWidth() * 3, 3 * 8,
		0, 0, 0);
	FreeImage_Save(fif, dib, filepath, 0);
	FreeImage_Unload(dib);
}
