#include "image.h"

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
