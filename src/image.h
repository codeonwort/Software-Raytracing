#pragma once

#include "type.h"
#include <vector>

struct Pixel
{
	float r;
	float g;
	float b;
	float a;

	Pixel()
	{
		r = g = b = 0.0f;
		a = 1.0f;
	}

	Pixel(float inR, float inG, float inB, float inA)
	{
		r = inR;
		g = inG;
		b = inB;
		a = inA;
	}

	Pixel(float inR, float inG, float inB)
	{
		r = inR;
		g = inG;
		b = inB;
		a = 1.0f;
	}

	Pixel(uint8 inR, uint8 inG, uint8 inB, uint8 inA)
	{
		r = (float)inR / 255.0f;
		g = (float)inG / 255.0f;
		b = (float)inB / 255.0f;
		a = (float)inA / 255.0f;
	}

	Pixel(uint32 argb)
	{
		uint32 A = (argb >> 24) & 0xff;
		uint32 R = (argb >> 16) & 0xff;
		uint32 G = (argb >> 8)  & 0xff;
		uint32 B = argb         & 0xff;
		a = (float)A / 255.0f;
		r = (float)R / 255.0f;
		g = (float)G / 255.0f;
		b = (float)B / 255.0f;
	}

	inline uint32 ToUint32() const
	{
		uint32 A = (uint32)(a * 255.0f) & 0xff;
		uint32 R = (uint32)(r * 255.0f) & 0xff;
		uint32 G = (uint32)(g * 255.0f) & 0xff;
		uint32 B = (uint32)(b * 255.0f) & 0xff;
		return (A << 24) | (R << 16) | (G << 8) | B;
	}

	inline vec3 RGBToVec3() const
	{
		return vec3(r, g, b);
	}

};

// Can be used as a 2D texture mipmap or a 2D render target
class Image2D
{

public:
	explicit Image2D();
	Image2D(uint32 width, uint32 height, const Pixel& argb);
	Image2D(uint32 width, uint32 height, uint32 color = 0xff000000);

	void Reallocate(uint32 width, uint32 height, const Pixel& clearColor = Pixel(0xff000000));

	void SetPixel(int32 x, int32 y, const Pixel& argb);
	void SetPixel(int32 x, int32 y, uint32 argb);

	void PostProcess(); // tone mapping, gamma correction, etc.

	inline uint32 GetWidth() const { return width; }
	inline uint32 GetHeight() const { return height; }
	inline Pixel GetPixel(int32 x, int32 y) const { return image[ix(x, y)]; }
	inline const std::vector<Pixel>& GetPixelArray() const { return image; }

//private:
public:
	inline int32 ix(int32 x, int32 y) const { return y * width + x; }

	uint32 width;
	uint32 height;
	std::vector<Pixel> image;

};
