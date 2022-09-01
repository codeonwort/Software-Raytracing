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
		const ray&       inPathRay,
		const HitResult& inHitResult,
		vec3&            outReflectance,
		ray&             outScatteredRay,
		float&           outPdf) const = 0;

	virtual vec3 Emitted(const HitResult& hitResult, const vec3& Wo) const
	{
		return vec3(0.0f, 0.0f, 0.0f);
	}

	// NOTE: Wo, Wi are in world space and Wo is outward from the surface.
	virtual float ScatteringPdf(
		const HitResult& hitResult,
		const vec3& Wo,
		const vec3& Wi) const
	{
		return 1.0f / BRDF::PI;
	}

};

class DiffuseLight : public Material
{

public:
	DiffuseLight(const vec3& inIntensity)
		: intensity(inIntensity)
	{
	}

	bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outReflectance, ray& outScatteredRay,
		float& outPdf) const override
	{
		return false;
	}

	vec3 Emitted(const HitResult& hitResult, const vec3& Wo) const override {
		return intensity;
	}

	vec3 intensity;

};

class Lambertian : public Material {
public:
	Lambertian(const vec3& inAlbedo) {
		albedo = saturate(inAlbedo);
	}

	bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outReflectance, ray& outScatteredRay,
		float& outPdf) const override;

	float ScatteringPdf(
		const HitResult& hitResult,
		const vec3& Wo,
		const vec3& Wi) const override;

private:
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

	bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outReflectance, ray& outScatteredRay,
		float& outPdf) const override;

	vec3 albedo;
	float fuzziness;

};

class Dielectric : public Material
{

public:
	Dielectric(float indexOfRefraction) : ref_idx(indexOfRefraction) {}

	bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outReflectance, ray& outScatteredRay,
		float& outPdf) const override;

	float ref_idx;

};

// Cook-Torrance BRDF
class MicrofacetMaterial : public Material {

public:
	MicrofacetMaterial()
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
	void SetEmissiveFallback(const vec3& inEmissive) { emissiveFallback = inEmissive; }

	bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outReflectance, ray& outScatteredRay,
		float& outPdf) const override;

	vec3 Emitted(const HitResult& hitResult, const vec3& Wo) const override;

	float ScatteringPdf(
		const HitResult& hitResult,
		const vec3& Wo,
		const vec3& Wi) const override;

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
