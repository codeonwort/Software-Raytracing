#include "texture.h"


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
	return Pixel(1.0f, 0.0f, 0.0f, 1.0f);
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
