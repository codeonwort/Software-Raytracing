#include "texture.h"
#include <algorithm>

Texture2D* Texture2D::CreateFromImage2D(const Image2D& inImage)
{
	Texture2D* texture = new Texture2D(1);
	texture->SetData(0, inImage);
	return texture;
}

Texture2D* Texture2D::CreateSolidColor(const Pixel& inColor)
{
	Image2D image;
	image.Reallocate(1, 1, inColor);
	Texture2D* texture = new Texture2D(1);
	texture->SetData(0, image);
	return texture;
}

Texture2D::Texture2D(uint32 numMipmaps)
{
	mipmaps.resize(numMipmaps);
}

void Texture2D::SetData(uint32 mipLevel, const Image2D& image)
{
	mipmaps[mipLevel] = image;
}

Pixel Texture2D::Sample(float u, float v)
{
	if (mipmaps.size() == 0)
	{
		return Pixel(0.0f, 0.0f, 0.0f, 0.0f);
	}

	// Fixup UV
	u = fmodf(u, 1.0f); if (u < 0.0f) u += 1.0f;
	v = fmodf(v, 1.0f); if (v < 0.0f) v += 1.0f; v = 1.0f - v;

	// #todo-texture: Filtering and wrapping
	const Image2D& image = mipmaps[0];
	int32 x = (int32)(image.GetWidth() * u);
	int32 y = (int32)(image.GetHeight() * v);
	x = std::max(0, std::min((int32)image.GetWidth() - 1, x));
	y = std::max(0, std::min((int32)image.GetHeight() - 1, y));
	Pixel px = image.GetPixel(x, y);
	if (sampler.bSRGB) {
		px = px.SRGBToLinear();
	}
	return px;
}

void Texture2D::FixUV(float& u, float& v)
{
	auto FixRange = [](float& x) -> void
	{
		if (x < 0.0 || x > 1.0)
		{
			//
		}
	};

	FixRange(u);
	FixRange(v);
}
