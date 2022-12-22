#include "texture.h"
#include <algorithm>

Texture2D* Texture2D::CreateFromImage2D(std::shared_ptr<Image2D> inImage)
{
	Texture2D* texture = new Texture2D(1);
	texture->SetData(0, inImage);
	return texture;
}

Texture2D* Texture2D::CreateSolidColor(const Pixel& inColor)
{
	std::shared_ptr<Image2D> image = std::make_shared<Image2D>();
	image->Reallocate(1, 1, inColor);
	Texture2D* texture = new Texture2D(1);
	texture->SetData(0, image);
	return texture;
}

Texture2D::Texture2D(uint32 numMipmaps)
{
	mipmaps.resize(numMipmaps);
}

void Texture2D::SetData(uint32 mipLevel, std::shared_ptr<Image2D> image)
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

	if (isnan(u) || isinf(u)) u = 0.0f;
	if (isnan(v) || isinf(v)) v = 0.0f;

	// #todo-texture: Filtering
	const Image2D& image = *(mipmaps[0].get());
	int32 x = (int32)((image.GetWidth() - 1) * u);
	int32 y = (int32)((image.GetHeight() - 1) * v);
	Pixel px = image.GetPixel(x, y);
	if (sampler.bSRGB) {
		px = px.SRGBToLinear();
	}
	return px;
}
