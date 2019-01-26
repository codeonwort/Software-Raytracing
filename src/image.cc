#include "image.h"

HDRImage::HDRImage(int32 inWidth, int32 inHeight, const Pixel& inPixel)
{
	width  = inWidth;
	height = inHeight;
	image.resize(width * height, inPixel);

	Pixel px = inPixel;
	uint8 r  = (int32)(px.r * 255.0f) & 0xff;
	uint8 g  = (int32)(px.g * 255.0f) & 0xff;
	uint8 b  = (int32)(px.b * 255.0f) & 0xff;
}

HDRImage::HDRImage(int32 inWidth, int32 inHeight, uint32 inColor)
	: HDRImage(inWidth, inHeight, Pixel(inColor))
{
}

void HDRImage::SetPixel(int32 x, int32 y, const Pixel& pixel)
{
	image[ix(x, y)] = pixel;
}

void HDRImage::SetPixel(int32 x, int32 y, uint32 rgb)
{
	image[ix(x, y)] = Pixel(rgb);
}

