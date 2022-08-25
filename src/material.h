#pragma once

#include "random.h"
#include "geom/ray.h"
#include "geom/hit.h"

#include "image.h"
#include "texture.h"

#include <algorithm>

#define SUPPORT_PBR_MATERIAL 1
// #todo-pbr: https://computergraphics.stackexchange.com/questions/4394/path-tracing-the-cook-torrance-brdf
#define FIX_BRDF             1

namespace BRDF
{
	const float PI = 3.14159265359f;

	inline vec3 FresnelSchlick(float cosTheta, vec3 F0)
	{
		return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
	}

	inline vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
	{
		return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
	}

	inline float DistributionGGX(vec3 N, vec3 H, float roughness)
	{
#if FIX_BRDF
		float a = roughness * roughness;
		float a2 = a * a;
		float NdotH = dot(N, H);
		float NdotH2 = NdotH * NdotH;

		float num = expf((NdotH2 - 1.0f) / (a2 * NdotH2));
		float denom = BRDF::PI * a2 * NdotH2 * NdotH2;
		return num / denom;
#else
		float a = roughness * roughness;
		float a2 = a * a;
		float NdotH = std::max(dot(N, H), 0.0f);
		float NdotH2 = NdotH * NdotH;

		float num = a2;
		float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
		denom = PI * denom * denom;

		return num / denom;
#endif
	}

	inline float GeometrySchlickGGX(float NdotV, float roughness)
	{
		float r = (roughness + 1.0f);
		float k = (r * r) / 8.0f;

		float num = NdotV;
		float denom = NdotV * (1.0f - k) + k;

		return num / denom;
	}

	inline float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
	{
		float NdotV = std::max(dot(N, V), 0.0f);
		float NdotL = std::max(dot(N, L), 0.0f);
		float ggx2 = GeometrySchlickGGX(NdotV, roughness);
		float ggx1 = GeometrySchlickGGX(NdotL, roughness);

		return ggx1 * ggx2;
	}
};

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

// #todo-pbr: Change to PBRMaterial and support other properties
class PBRMaterial : public Material
{

public:
	PBRMaterial()
		: albedoTexture(nullptr)
		, normalmapTexture(nullptr)
		, roughnessTexture(nullptr)
		, metallicTexture(nullptr)
		, emissiveTexture(nullptr)
	{
	}

	void SetAlbedoTexture(const Image2D& inImage)
	{
		if (albedoTexture) delete albedoTexture;
		albedoTexture = Texture2D::CreateFromImage2D(inImage);
	}
	void SetNormalTexture(const Image2D& inImage)
	{
		if (normalmapTexture) delete normalmapTexture;
		normalmapTexture = Texture2D::CreateFromImage2D(inImage);
	}
	void SetRoughnessTexture(const Image2D& inImage)
	{
		if (roughnessTexture) delete roughnessTexture;
		roughnessTexture = Texture2D::CreateFromImage2D(inImage);
	}
	void SetMetallicTexture(const Image2D& inImage)
	{
		if (metallicTexture) delete metallicTexture;
		metallicTexture = Texture2D::CreateFromImage2D(inImage);
	}
	void SetEmissiveTexture(const Image2D& inImage)
	{
		if (emissiveTexture) delete emissiveTexture;
		emissiveTexture = Texture2D::CreateFromImage2D(inImage);
	}

	virtual bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outAttenuation, ray& outScattered) const override
	{
		vec3 albedo(0.0f, 0.0f, 0.0f);
		vec3 Wo = normalize(RandomInHemisphere(inResult.n));
		
		// #todo-texture: Pre-multiply alpha?
		if (albedoTexture)
		{
			albedo = albedoTexture->Sample(inResult.paramU, inResult.paramV).RGBToVec3();
		}

#if SUPPORT_PBR_MATERIAL
		// Default: Lambertian non-metal surface
		float roughness = 1.0f;
		float metallic = 0.0f;

		vec3 N = inResult.n;
		vec3 V = -inRay.d;
		vec3 H = normalize(V + Wo);
		if (normalmapTexture)
		{
			vec3 localN = normalmapTexture->Sample(inResult.paramU, inResult.paramV).RGBToVec3();
			// #todo-pbr: Rotate localN around N (normal mapping)
		}
		if (roughnessTexture)
		{
			roughness = roughnessTexture->Sample(inResult.paramU, inResult.paramV).r;
		}
		if (metallicTexture)
		{
			metallic = metallicTexture->Sample(inResult.paramU, inResult.paramV).r;
		}

		vec3 F0 = vec3(0.04f);
		F0 = mix(F0, min(albedo, vec3(1.0f)), metallic);

		float NDF = BRDF::DistributionGGX(N, H, roughness);
		float G = BRDF::GeometrySmith(N, V, Wo, roughness);
		vec3 F = BRDF::FresnelSchlick(std::max(dot(H, V), 0.0f), F0);

#if 0
		vec3 kS = F;
#else
		// #todo-pbr: How to sample specular component
		// when I randomly select the outgoing direction?
		// Let's cancel out for now.
		vec3 kS = vec3(0.0f);
#endif
		vec3 kD = vec3(1.0f) - kS;
		kD *= 1.0f - metallic;
		vec3 diffuse = kD * albedo;

		// #todo-pbr: Is this right?
		float NdotL = std::max(dot(N, Wo), 0.0f);
		float NdotV = std::max(dot(N, V), 0.0f);

#if FIX_BRDF
		vec3 specular = (BRDF::PI * 0.5f) * (NDF * F * G) / NdotL;
#else
		vec3 num = NDF * G * F;
		float denom = 4.0f * NdotV * NdotL;
		vec3 specular = num / max(denom, 0.001f);
#endif

		outScattered = ray(inResult.p, Wo, inRay.t);
		outAttenuation = (kD * diffuse + kS * specular) * NdotL;
		return true;
#else // SUPPORT_PBR_MATERIAL
		outScattered = ray(inResult.p, Wo, inRay.t);
		outAttenuation = albedo;
		return true;
#endif // SUPPORT_PBR_MATERIAL
	}

	virtual vec3 Emitted(float u, float v, const vec3& inPosition) const
	{
		if (emissiveTexture)
		{
			Pixel emissiveSample = emissiveTexture->Sample(u, v);
			return vec3(emissiveSample.r, emissiveSample.g, emissiveSample.b);
		}
		return vec3(0.0f, 0.0f, 0.0f);
	}

private:
	Texture2D* albedoTexture;
	Texture2D* normalmapTexture;
	Texture2D* roughnessTexture;
	Texture2D* metallicTexture;
	Texture2D* emissiveTexture;

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
		vec3& outAttenuation, ray& outScattered) const override
	{
		vec3 ud = inRay.d;
		ud.Normalize();
		vec3 reflected = reflect(ud, inResult.n);
		outScattered = ray(inResult.p, reflected + fuzziness * RandomInUnitSphere(), inRay.t);
		outAttenuation = albedo;
		return (dot(outScattered.d, inResult.n) > 0.0f);
	}

	vec3 albedo;
	float fuzziness;

};

inline float Schlick(float cosine, float ref_idx)
{
	float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
	r0 = r0 * r0;
	return r0 + (1.0f - r0) * pow((1.0f - cosine), 5.0f);
}

class Dielectric : public Material
{

public:
	Dielectric(float indexOfRefraction) : ref_idx(indexOfRefraction) {}

	virtual bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outAttenuation, ray& outScattered) const override
	{
		vec3 outward_normal;
		vec3 reflected = reflect(inRay.d, inResult.n);
		float ni_over_nt;
		outAttenuation = vec3(1.0f, 1.0f, 1.0f);
		vec3 refracted;
		float reflect_prob;
		float cosine;
		if(dot(inRay.d, inResult.n) > 0.0f)
		{
			outward_normal = -inResult.n;
			ni_over_nt = ref_idx;
			cosine = ref_idx * dot(inRay.d, inResult.n) / inRay.d.Length();
		}
		else
		{
			outward_normal = inResult.n;
			ni_over_nt = 1.0f / ref_idx;
			cosine = -dot(inRay.d, inResult.n) / inRay.d.Length();
		}
		if(refract(inRay.d, outward_normal, ni_over_nt, refracted))
		{
			reflect_prob = Schlick(cosine, ref_idx);
		}
		else
		{
			reflect_prob = 1.0f;
		}
		if(Random() < reflect_prob)
		{
			outScattered = ray(inResult.p, reflected, inRay.t);
		}
		else
		{
			outScattered = ray(inResult.p, refracted, inRay.t);
		}
		return true;
	}

	float ref_idx;

};

