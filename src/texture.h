#pragma once

#include "type.h"
#include "image.h"
#include "template/noncopyable.h"

#include <vector>


enum class ETextureFilter : uint8
{
	Nearest,
	Linear
};

enum class ETextureWrap : uint8
{
	Clamp,
	Repeat
};

struct SamplerState
{
	SamplerState()
		: filter(ETextureFilter::Linear)
		, wrap(ETextureWrap::Repeat)
	{
	}

	ETextureFilter filter;
	ETextureWrap wrap;
};

// Collection of mipmaps
class Texture2D : public Noncopyable
{

public:
	// Create a texture from single mipmap
	static Texture2D* CreateFromImage2D(const Image2D& inImage);

	static Texture2D* CreateSolidColor(const Pixel& inColor);
	
public:
	Texture2D(uint32 numMipmaps);

	void SetData(uint32 mipLevel, const Image2D& image);
	void SetSamplerState(const SamplerState& inSampler) { sampler = inSampler; }

	Pixel Sample(float u, float v);

private:
	void FixUV(float& u, float& v);

	std::vector<Image2D> mipmaps;
	SamplerState sampler;

};
