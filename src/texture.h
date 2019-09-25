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
	ETextureFilter filter;
	ETextureWrap wrap;
};

class Texture2D : public Noncopyable
{
	
public:
	Texture2D(uint32 width, uint32 height);

	void Clear(const Pixel& texel);
	void SetData(const std::vector<Pixel>& inData);
	void SetSamplerState(const SamplerState& inSampler) { sampler = inSampler; }

	Pixel Sample(float u, float v);

private:
	void FixUV(float& u, float& v);

	std::vector<Pixel> data;
	SamplerState sampler;

};
