#pragma once

#include "raylib_types.h"
#include "core/int_types.h"
#include "core/vec3.h"
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

	// Assumes current values are in linear color space.
	inline Pixel LinearToSRGB()
	{
		const float K = 1.0f / 2.2f;
		return Pixel{ powf(r, K), powf(g, K), powf(b, K), powf(a, K) };
	}

	// Assumes current values are in sRGB color space.
	inline Pixel SRGBToLinear()
	{
		const float K = 2.2f;
		return Pixel{ powf(r, K), powf(g, K), powf(b, K), powf(a, K) };
	}

};

// Can be used as a 2D texture mipmap or a 2D render target
class Image2D
{

public:
	RAYLIB_API explicit Image2D();
	RAYLIB_API Image2D(uint32 width, uint32 height, const Pixel& argb);
	RAYLIB_API Image2D(uint32 width, uint32 height, uint32 color = 0xff000000);

	RAYLIB_API void Reallocate(uint32 width, uint32 height, const Pixel& clearColor = Pixel(0xff000000));

	RAYLIB_API void SetPixel(int32 x, int32 y, const Pixel& argb);
	RAYLIB_API void SetPixel(int32 x, int32 y, uint32 argb);

	RAYLIB_API void PostProcess(); // tone mapping, gamma correction, etc.

	inline uint32 GetWidth() const { return width; }
	inline uint32 GetHeight() const { return height; }
	inline Pixel GetPixel(int32 x, int32 y) const { return image[ix(x, y)]; }
	inline const std::vector<Pixel>& GetPixelArray() const { return image; }

	Image2D Clone() const;
	RAYLIB_API void DumpFloatRGBs(std::vector<float>& outArray);

private:
	inline int32 ix(int32 x, int32 y) const { return y * width + x; }

	uint32 width;
	uint32 height;
	std::vector<Pixel> image; // row-major

};
