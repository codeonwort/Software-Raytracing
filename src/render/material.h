#pragma once

#include "core/random.h"
#include "render/brdf.h"
#include "render/image.h"
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

	virtual vec3 GetAlbedo(float paramU, float paramV) const { return vec3(0.0f); }
	virtual bool AlphaTest(float paramU, float paramV) const { return true; }
	// In local tangent space
	virtual vec3 GetMicrosurfaceNormal(const HitResult& hitResult) const { return vec3(0.0f, 0.0f, 1.0f); }
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

	virtual vec3 GetAlbedo(float paramU, float paramV) const override { return albedo; }

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

	virtual vec3 GetAlbedo(float paramU, float paramV) const { return albedo; }

	vec3 albedo;
	float fuzziness;
};

class Dielectric : public Material
{

public:
	Dielectric(float indexOfRefraction, const vec3& inTransmissionFilter = vec3(1.0f))
		: ref_idx(indexOfRefraction)
		, transmissionFilter(inTransmissionFilter)
	{}

	bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outReflectance, ray& outScatteredRay,
		float& outPdf) const override;

	float ref_idx;
	vec3 transmissionFilter;
};

class Mirror : public Material
{
public:
	Mirror(const vec3& inBaseColor = vec3(1.0f))
		: baseColor(inBaseColor)
	{}

	virtual bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outReflectance, ray& outScatteredRay,
		float& outPdf) const override
	{
		outReflectance = baseColor;
		outScatteredRay = ray(inResult.p, reflect(inRay.d, inResult.n), inRay.t);
		outPdf = 1.0f;
		return true;
	}

	virtual float ScatteringPdf(const HitResult& hitResult, const vec3& Wo, const vec3& Wi) const
	{
		return 1.0f;
	}

	vec3 baseColor;
};

class MicrofacetMaterial : public Material {

public:
	static MicrofacetMaterial* FromConstants(
		const vec3& inAlbedo,
		const float inRoughness,
		const float inMetallic,
		const vec3& inEmissive)
	{
		MicrofacetMaterial* M = new MicrofacetMaterial;
		M->albedoFallback = inAlbedo;
		M->roughnessFallback = inRoughness;
		M->metallicFallback = inMetallic;
		M->emissiveFallback = inEmissive;
		return M;
	}

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

	void SetAlbedoTexture(std::shared_ptr<Image2D> inImage) {
		if (albedoTexture) delete albedoTexture;
		albedoTexture = Texture2D::CreateFromImage2D(inImage);

		SamplerState sampler;
		sampler.bSRGB = true;
		albedoTexture->SetSamplerState(sampler);
	}
	void SetNormalTexture(std::shared_ptr<Image2D> inImage) {
		if (normalmapTexture) delete normalmapTexture;
		normalmapTexture = Texture2D::CreateFromImage2D(inImage);
	}
	void SetRoughnessTexture(std::shared_ptr<Image2D> inImage) {
		if (roughnessTexture) delete roughnessTexture;
		roughnessTexture = Texture2D::CreateFromImage2D(inImage);
	}
	void SetMetallicTexture(std::shared_ptr<Image2D> inImage) {
		if (metallicTexture) delete metallicTexture;
		metallicTexture = Texture2D::CreateFromImage2D(inImage);
	}
	void SetEmissiveTexture(std::shared_ptr<Image2D> inImage) {
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

	virtual vec3 GetAlbedo(float paramU, float paramV) const override;
	virtual bool AlphaTest(float texcoordU, float texcoordV) const override;
	virtual vec3 GetMicrosurfaceNormal(const HitResult& hitResult) const override;

private:
	// wi = reflect(-wo, wh)
	vec3 Sample_wh(const vec3& wo, float alpha) const;

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
