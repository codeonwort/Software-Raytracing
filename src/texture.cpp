#include "texture.h"


Texture2D::Texture2D(uint32 width, uint32 height)
{
	//
}

void Texture2D::Clear(const Pixel& texel)
{

}

void Texture2D::SetData(const std::vector<Pixel>& inData)
{

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
