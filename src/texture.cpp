#include "texture.h"
#include <algorithm>

Texture2D* Texture2D::CreateFromImage2D(const Image2D& inImage)
{
	Texture2D* texture = new Texture2D(1);
	texture->SetData(0, inImage);
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
	// #todo: Filtering and wrapping
	const Image2D& image = mipmaps[0];
	int32 x = (int32)(image.GetWidth() * u);
	int32 y = (int32)(image.GetHeight() * v);
	x = std::max(0, std::min((int32)image.GetWidth() - 1, x));
	y = std::max(0, std::min((int32)image.GetHeight() - 1, y));
	return image.GetPixel(x, y);
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
