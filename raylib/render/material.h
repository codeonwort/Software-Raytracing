#pragma once

#include "raylib_types.h"
#include "core/random.h"
#include "render/brdf.h"
#include "render/image.h"
#include "render/texture.h"
#include "geom/ray.h"
#include "geom/hit.h"

#include <algorithm>

// #todo-raylib: Remove RAYLIB_API

// Base class for all materials
class Material
{

public:
	// 1. Produce a scattered ray
	// 2. If scattered, tell how much the ray should be attenuated
	RAYLIB_API virtual bool Scatter(
		const ray&       inPathRay,
		const HitResult& inHitResult,
		vec3&            outReflectance,
		ray&             outScatteredRay,
		float&           outPdf) const = 0;

	RAYLIB_API virtual vec3 Emitted(const HitResult& hitResult, const vec3& Wo) const
	{
		return vec3(0.0f, 0.0f, 0.0f);
	}

	// NOTE: Wo, Wi are in world space and Wo is outward from the surface.
	RAYLIB_API virtual float ScatteringPdf(
		const HitResult& hitResult,
		const vec3& Wo,
		const vec3& Wi) const
	{
		return 1.0f / BRDF::PI;
	}

	virtual bool IsMirrorLike(float paramU, float paramV) const { return false; }
	RAYLIB_API virtual vec3 GetAlbedo(float paramU, float paramV) const { return vec3(0.0f); }
	RAYLIB_API virtual bool AlphaTest(float paramU, float paramV) const { return true; }
	// In local tangent space
	RAYLIB_API virtual vec3 GetMicrosurfaceNormal(const HitResult& hitResult) const { return vec3(0.0f, 0.0f, 1.0f); }
};

class DiffuseLight : public Material
{

public:
	RAYLIB_API DiffuseLight(const vec3& inIntensity)
		: intensity(inIntensity)
	{
	}

	RAYLIB_API bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outReflectance, ray& outScatteredRay,
		float& outPdf) const override
	{
		return false;
	}

	RAYLIB_API vec3 Emitted(const HitResult& hitResult, const vec3& Wo) const override {
		return intensity;
	}

public:
	vec3 intensity;

};

class Lambertian : public Material
{
public:
	RAYLIB_API Lambertian(const vec3& inAlbedo)
	{
		albedo = saturate(inAlbedo);
	}

	RAYLIB_API bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outReflectance, ray& outScatteredRay,
		float& outPdf) const override;

	RAYLIB_API float ScatteringPdf(
		const HitResult& hitResult,
		const vec3& Wo,
		const vec3& Wi) const override;

	RAYLIB_API virtual vec3 GetAlbedo(float paramU, float paramV) const override { return albedo; }

private:
	vec3 albedo;
};

class Metal : public Material
{
public:
	RAYLIB_API Metal(const vec3& inAlbedo, float inFuzziness = 0.0f)
		: albedo(inAlbedo)
		, fuzziness(inFuzziness)
	{
		fuzziness = std::max(0.0f, std::min(1.0f, fuzziness));
	}

	RAYLIB_API bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outReflectance, ray& outScatteredRay,
		float& outPdf) const override;

	RAYLIB_API virtual vec3 GetAlbedo(float paramU, float paramV) const { return albedo; }

public:
	vec3 albedo;
	float fuzziness;
};

class Dielectric : public Material
{
public:
	RAYLIB_API Dielectric(float indexOfRefraction, const vec3& inTransmissionFilter = vec3(1.0f))
		: ref_idx(indexOfRefraction)
		, transmissionFilter(inTransmissionFilter)
	{}

	RAYLIB_API bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outReflectance, ray& outScatteredRay,
		float& outPdf) const override;

	virtual bool IsMirrorLike(float paramU, float paramV) const override { return true; }

public:
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

	virtual bool IsMirrorLike(float paramU, float paramV) const override { return true; }

public:
	vec3 baseColor;
};

class MicrofacetMaterial : public Material
{
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

	RAYLIB_API MicrofacetMaterial()
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

	RAYLIB_API void SetAlbedoTexture(std::shared_ptr<Image2D> inImage)
	{
		if (albedoTexture) delete albedoTexture;
		albedoTexture = Texture2D::CreateFromImage2D(inImage);

		SamplerState sampler;
		sampler.bSRGB = true;
		albedoTexture->SetSamplerState(sampler);
	}
	RAYLIB_API void SetNormalTexture(std::shared_ptr<Image2D> inImage)
	{
		if (normalmapTexture) delete normalmapTexture;
		normalmapTexture = Texture2D::CreateFromImage2D(inImage);
	}
	RAYLIB_API void SetRoughnessTexture(std::shared_ptr<Image2D> inImage)
	{
		if (roughnessTexture) delete roughnessTexture;
		roughnessTexture = Texture2D::CreateFromImage2D(inImage);
	}
	RAYLIB_API void SetMetallicTexture(std::shared_ptr<Image2D> inImage)
	{
		if (metallicTexture) delete metallicTexture;
		metallicTexture = Texture2D::CreateFromImage2D(inImage);
	}
	RAYLIB_API void SetEmissiveTexture(std::shared_ptr<Image2D> inImage)
	{
		if (emissiveTexture) delete emissiveTexture;
		emissiveTexture = Texture2D::CreateFromImage2D(inImage);
	}

	void SetAlbedoFallback(const vec3& inAlbedo) { albedoFallback = saturate(inAlbedo); }
	void SetRoughnessFallback(float inRoughness) { roughnessFallback = std::min(1.0f, std::max(0.0f, inRoughness)); }
	void SetMetallicFallback(float inMetallic) { metallicFallback = std::min(1.0f, std::max(0.0f, inMetallic)); }
	void SetEmissiveFallback(const vec3& inEmissive) { emissiveFallback = inEmissive; }

	RAYLIB_API bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outReflectance, ray& outScatteredRay,
		float& outPdf) const override;

	RAYLIB_API vec3 Emitted(const HitResult& hitResult, const vec3& Wo) const override;

	RAYLIB_API float ScatteringPdf(
		const HitResult& hitResult,
		const vec3& Wo,
		const vec3& Wi) const override;

	virtual bool IsMirrorLike(float paramU, float paramV) const override;

	RAYLIB_API virtual vec3 GetAlbedo(float paramU, float paramV) const override;
	RAYLIB_API virtual bool AlphaTest(float texcoordU, float texcoordV) const override;
	RAYLIB_API virtual vec3 GetMicrosurfaceNormal(const HitResult& hitResult) const override;

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
