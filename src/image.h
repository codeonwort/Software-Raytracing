#pragma once

#include "type.h"
#include <vector>

struct Pixel
{
	float r;
	float g;
	float b;

	Pixel() { r = g = b = 0.0f; }

	Pixel(float inR, float inG, float inB)
	{
		r = inR;
		g = inG;
		b = inB;
	}

	Pixel(uint32 rgb)
	{
		uint32 R = (rgb >> 16) & 0xff;
		uint32 G = (rgb >> 8)  & 0xff;
		uint32 B = rgb         & 0xff;
		r = (float)R / 255.0f;
		g = (float)G / 255.0f;
		b = (float)B / 255.0f;
	}

	inline uint32 ToUint32() const
	{
		uint32 R = (uint32)(r * 255.0f) & 0xff;
		uint32 G = (uint32)(g * 255.0f) & 0xff;
		uint32 B = (uint32)(b * 255.0f) & 0xff;
		return (R << 16) | (G << 8) | B;
	}

};

class HDRImage
{

public:
	HDRImage(int32 width, int32 height, const Pixel& rgb);
	HDRImage(int32 width, int32 height, uint32 color = 0x00000000);

	void SetPixel(int32 x, int32 y, const Pixel& rgb);
	void SetPixel(int32 x, int32 y, uint32 rgb);

	inline int32 GetWidth() const { return width; }
	inline int32 GetHeight() const { return height; }
	inline Pixel GetPixel(int32 x, int32 y) const { return image[ix(x, y)]; }

//private:
public:
	inline int32 ix(int32 x, int32 y) const { return y * width + x; }

	int32 width;
	int32 height;
	std::vector<Pixel> image;

};

