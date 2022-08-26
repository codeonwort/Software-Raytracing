#pragma once

#include "random.h"
#include "brdf.h"
#include "image.h"
#include "texture.h"
#include "geom/ray.h"
#include "geom/hit.h"

#include <algorithm>

// Base class for all materials
class Material
{

public:
	// 1. Produce a scattered ray
	// 2. If scattered, tell how much the ray should be attenuated
	virtual bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outAttenuation, ray& outScattered) const = 0;

	// u,v : surface parameterization
	// inPosition : hit point
	virtual vec3 Emitted(float u, float v, const vec3& inPosition) const
	{
		return vec3(0.0f, 0.0f, 0.0f);
	}

};

class DiffuseLight : public Material
{

public:
	DiffuseLight(const vec3& inIntensity)
		: intensity(inIntensity)
	{
	}

	virtual bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outAttenuation, ray& outScattered) const
	{
		return false;
	}

	virtual vec3 Emitted(float u, float v, const vec3& inPosition) const
	{
		return intensity;
	}

	vec3 intensity;

};

class Lambertian : public Material
{

public:
	Lambertian(const vec3& inAlbedo)
		: albedo(inAlbedo)
	{
	}

	virtual bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outAttenuation, ray& outScattered) const override
	{
		outScattered = ray(inResult.p, RandomInHemisphere(inResult.n), inRay.t);
		outAttenuation = albedo;
		return true;
	}

	vec3 albedo;

};

class Metal : public Material
{

public:
	Metal(const vec3& inAlbedo, float inFuzziness = 0.0f)
		: albedo(inAlbedo)
		, fuzziness(inFuzziness)
	{
		fuzziness = std::max(0.0f, std::min(1.0f, fuzziness));
	}

	virtual bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outAttenuation, ray& outScattered) const override;

	vec3 albedo;
	float fuzziness;

};

class Dielectric : public Material
{

public:
	Dielectric(float indexOfRefraction) : ref_idx(indexOfRefraction) {}

	virtual bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outAttenuation, ray& outScattered) const override;

	float ref_idx;

};

class PBRMaterial : public Material {

public:
	PBRMaterial()
		: albedoTexture(nullptr)
		, normalmapTexture(nullptr)
		, roughnessTexture(nullptr)
		, metallicTexture(nullptr)
		, emissiveTexture(nullptr)
		, albedoFallback(vec3(0.5f, 0.5f, 0.5f))
		, roughnessFallback(1.0f)
		, metallicFallback(0.0f)
		, emissiveFallback(vec3(0.0f, 0.0f, 0.0f))
	{
	}

	void SetAlbedoTexture(const Image2D& inImage) {
		if (albedoTexture) delete albedoTexture;
		albedoTexture = Texture2D::CreateFromImage2D(inImage);
	}
	void SetNormalTexture(const Image2D& inImage) {
		if (normalmapTexture) delete normalmapTexture;
		normalmapTexture = Texture2D::CreateFromImage2D(inImage);
	}
	void SetRoughnessTexture(const Image2D& inImage) {
		if (roughnessTexture) delete roughnessTexture;
		roughnessTexture = Texture2D::CreateFromImage2D(inImage);
	}
	void SetMetallicTexture(const Image2D& inImage) {
		if (metallicTexture) delete metallicTexture;
		metallicTexture = Texture2D::CreateFromImage2D(inImage);
	}
	void SetEmissiveTexture(const Image2D& inImage) {
		if (emissiveTexture) delete emissiveTexture;
		emissiveTexture = Texture2D::CreateFromImage2D(inImage);
	}

	void SetAlbedoFallback(const vec3& inAlbedo) { albedoFallback = saturate(inAlbedo); }
	void SetRoughnessFallback(float inRoughness) { roughnessFallback = std::min(1.0f, std::max(0.0f, inRoughness)); }
	void SetMetallicFallback(float inMetallic) { metallicFallback = std::min(1.0f, std::max(0.0f, inMetallic)); }
	void SetEmissiveFallback(const vec3& inEmissive) { emissiveFallback = saturate(inEmissive); }

	virtual bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outAttenuation, ray& outScattered) const override;

	virtual vec3 Emitted(float u, float v, const vec3& inPosition) const;

private:
	Texture2D* albedoTexture;
	Texture2D* normalmapTexture;
	Texture2D* roughnessTexture;
	Texture2D* metallicTexture;
	Texture2D* emissiveTexture;

	// Used when relevant texture is absent.
	vec3 albedoFallback;
	float roughnessFallback;
	float metallicFallback;
	vec3 emissiveFallback;
};
