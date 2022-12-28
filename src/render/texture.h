#pragma once

#include "render/image.h"
#include "core/int_types.h"
#include "core/noncopyable.h"

#include <vector>
#include <memory>

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
		, bSRGB(false)
	{
	}

	ETextureFilter filter;
	ETextureWrap wrap;
	bool bSRGB;
};

// Collection of mipmaps
class Texture2D : public Noncopyable
{

public:
	// Create a texture from single mipmap
	static Texture2D* CreateFromImage2D(std::shared_ptr<Image2D> inImage);

	static Texture2D* CreateSolidColor(const Pixel& inColor);
	
public:
	Texture2D(uint32 numMipmaps);

	void SetData(uint32 mipLevel, std::shared_ptr<Image2D> image);
	void SetSamplerState(const SamplerState& inSampler) { sampler = inSampler; }

	Pixel Sample(float u, float v);

private:
	std::vector<std::shared_ptr<Image2D>> mipmaps;
	SamplerState sampler;

};
