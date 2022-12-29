#include "image.h"
#include "core/int_types.h"
#include "core/vec3.h"
#include "core/logger.h"

#pragma warning(push)
#pragma warning(disable: 4819)
#include "FreeImage.h"
#pragma warning(pop)

#define TONE_MAP         1    // Still some artifact around borders that I don't quite get
#define FORCE_MAX_WHITE  1    // Clamp the tone mapping result to white
#define GAMMA_CORRECTION 1    // linear to sRGB
#define GAMMA_VALUE      2.2f

Image2D::Image2D()
{
	Reallocate(0, 0);
}

Image2D::Image2D(uint32 inWidth, uint32 inHeight, const Pixel& inPixel)
{
	Reallocate(inWidth, inHeight, inPixel);
}

Image2D::Image2D(uint32 inWidth, uint32 inHeight, uint32 inColor)
	: Image2D(inWidth, inHeight, Pixel(inColor))
{
}

void Image2D::Reallocate(uint32 inWidth, uint32 inHeight, const Pixel& clearColor /*= Pixel(0xff000000)*/)
{
	width = inWidth;
	height = inHeight;
	image.resize(width * height, clearColor);
}

void Image2D::SetPixel(int32 x, int32 y, const Pixel& pixel)
{
	image[ix(x, y)] = pixel;
}

void Image2D::SetPixel(int32 x, int32 y, uint32 argb)
{
	image[ix(x, y)] = Pixel(argb);
}

void Image2D::PostProcess()
{
	// https://64.github.io/tonemapping/
	auto Luminance = [](const vec3& v) -> float
	{
		return dot(v, vec3(0.2126f, 0.7152f, 0.0722f));
	};
	auto LuminanceToneMap = [&](const vec3& v, float maxWhiteLuminance) -> vec3
	{
		float luminanceOld = Luminance(v);
		// Div by zero if progress any further
		if (luminanceOld <= 0.0001f)
		{
			return vec3(0.0f, 0.0f, 0.0f);
		}
		float numerator = luminanceOld * (1.0f + (luminanceOld / (maxWhiteLuminance * maxWhiteLuminance)));
		float luminanceNew = numerator / (1.0f + luminanceOld);
		return v * (luminanceNew / luminanceOld);
	};

	const size_t len = (size_t)(width * height);
	float maxWhiteLuminance = 1.0f;

	for (size_t i = 0; i < len; ++i)
	{
		//maxWhiteLuminance = std::max(maxWhiteLuminance, std::max(image[i].r, std::max(image[i].g, image[i].b)));
		float L = Luminance(vec3(image[i].r, image[i].g, image[i].b));
		if (maxWhiteLuminance < L)
		{
			maxWhiteLuminance = L;
		}
	}

	LOG("Max white luminance: %f", maxWhiteLuminance);

	for (size_t i = 0; i < len; ++i)
	{
		vec3 rgb(image[i].r, image[i].g, image[i].b);

#if TONE_MAP
		// Extended Reinhard (Luminance Tone Map)
		rgb = LuminanceToneMap(rgb, maxWhiteLuminance);
#endif

#if FORCE_MAX_WHITE
		// Still needs to clamp to white?
		rgb = min(vec3(1.0f, 1.0f, 1.0f), rgb);
#endif

		// Gamma correction
#if GAMMA_CORRECTION
		rgb = pow(rgb, 1.0f / GAMMA_VALUE);
#endif
		
		// Final output
		image[i].r = rgb.x;
		image[i].g = rgb.y;
		image[i].b = rgb.z;
	}
}

Image2D Image2D::Clone() const
{
	Image2D img{};
	img.width = width;
	img.height = height;
	img.image = image;
	return img;
}

void Image2D::DumpFloatRGBs(std::vector<float>& outArray)
{
	outArray.clear();
	outArray.reserve(3 * width * height);

	size_t k = 0;
	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			outArray.push_back(image[k].r);
			outArray.push_back(image[k].g);
			outArray.push_back(image[k].b);
			++k;
		}
	}
}

// -----------------------------------------------------------------------

static FREE_IMAGE_FORMAT ToFreeImageType(EImageFileType inType)
{
	switch (inType)
	{
		case RAYLIB_IMAGEFILETYPE_Bitmap: return FIF_BMP;
		case RAYLIB_IMAGEFILETYPE_Jpg:    return FIF_JPEG;
		case RAYLIB_IMAGEFILETYPE_Png:    return FIF_PNG;
	}
	CHECKF(false, "Unexpected EImageFileType value");
	return FIF_UNKNOWN;
}

namespace ImageIO
{
	void InitializeImageIO()
	{
		FreeImage_Initialise();
	}

	void TerminateImageIO()
	{
		FreeImage_DeInitialise();
	}

	Image2D* LoadImage2DFromFile(const char* filepath)
	{
		if (filepath == nullptr)
		{
			return nullptr;
		}
	
		FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFIFFromFilename(filepath);
		FIBITMAP* dib = FreeImage_Load(imageFormat, filepath, 0);

		if (dib == nullptr)
		{
			return nullptr;
		}

		Image2D* image = new Image2D;

		if (imageFormat == FIF_HDR)
		{
			FIBITMAP* dibF = FreeImage_ConvertToRGBAF(dib);
			FreeImage_Unload(dib);

			float* hdrBuffer = reinterpret_cast<float*>(FreeImage_GetBits(dibF));
			uint32 width = (uint32)FreeImage_GetWidth(dibF);
			uint32 height = (uint32)FreeImage_GetHeight(dibF);
			uint32 pitch = (uint32)FreeImage_GetPitch(dibF);

			image->Reallocate(width, height);

			uint32 p = 0;
			for (int32 y = (int32)height - 1; y >= 0; --y)
			{
				for (int32 x = 0; x < (int32)width; ++x)
				{
					float r = hdrBuffer[p++];
					float g = hdrBuffer[p++];
					float b = hdrBuffer[p++];
					float a = hdrBuffer[p++];
					image->SetPixel(x, y, Pixel(r, g, b, a));
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

			image->Reallocate(width, height);

			uint32 p = 0;
			for (int32 y = (int32)height - 1; y >= 0; --y)
			{
				for (int32 x = 0; x < (int32)width; ++x)
				{
					uint8 b = colorBuffer[p++];
					uint8 g = colorBuffer[p++];
					uint8 r = colorBuffer[p++];
					uint8 a = colorBuffer[p++];
					image->SetPixel(x, y, Pixel(r, g, b, a));
				}
			}

			FreeImage_Unload(dib32);
		}

		return image;
	}

	bool WriteImage2DToDisk(Image2D* image, const char* filepath, EImageFileType fileType)
	{
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
		bool bSuccess = FreeImage_Save(fif, dib, filepath, 0);
		FreeImage_Unload(dib);

		return bSuccess;
	}
}
