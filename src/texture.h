#pragma once

#include "type.h"
#include "template/noncopyable.h"

#include <vector>

struct Texel
{
	Texel(float inR, float inG, float inB, float inA)
		: r(inR)
		, g(inG)
		, b(inB)
		, a(inA)
	{
	}
	float r;
	float g;
	float b;
	float a;
};

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

	void Clear(const Texel& texel);
	void SetData(const std::vector<Texel>& inData);
	void SetSamplerState(const SamplerState& inSampler) { sampler = inSampler; }

	Texel Sample(float u, float v);

private:
	void FixUV(float& u, float& v);

	std::vector<Texel> data;
	SamplerState sampler;

};
