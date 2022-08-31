#include "image.h"
#include "type.h"
#include "log.h"
#include "core/vec.h"

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

	log("Max white luminance: %f", maxWhiteLuminance);

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
