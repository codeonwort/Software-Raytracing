#pragma once

#include "random.h"
#include "geom/ray.h"
#include "geom/hit.h"

#include "image.h"
#include "texture.h"

#include <algorithm>

// #todo-pbr: References
// https://computergraphics.stackexchange.com/questions/4394/path-tracing-the-cook-torrance-brdf
// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
#define SUPPORT_PBR_MATERIAL 1

namespace BRDF
{
	const float PI = 3.14159265359f;

	// cosTheta = dot(incident_or_exitant_light, half_vector)
	inline vec3 FresnelSchlick(float cosTheta, const vec3& F0)
	{
		return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
	}

	inline vec3 FresnelSchlickRoughness(float cosTheta, const vec3& F0, float roughness)
	{
		return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
	}

	inline float DistributionGGX(const vec3& N, const vec3& H, float roughness)
	{
		float a = roughness * roughness;
		float a2 = a * a;
		float NdotH = dot(N, H);
		float NdotH2 = NdotH * NdotH;

		float num = a2;
		float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
		denom = PI * denom * denom;

		return num / denom;
	}

	inline float GeometrySchlickGGX(float NdotV, float roughness)
	{
		float r = (roughness + 1.0f);
		float k = (r * r) / 8.0f;

		float num = NdotV;
		float denom = NdotV * (1.0f - k) + k;

		return num / denom;
	}

	inline float GeometrySmith(const vec3& N, const vec3& V, const vec3& L, float roughness)
	{
		float NdotV = std::max(0.0f, dot(N, V));
		float NdotL = std::max(0.0f, dot(N, L));
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
		vec3 baseColor(0.0f, 0.0f, 0.0f);
		
		// #todo-texture: Pre-multiply alpha?
		if (albedoTexture)
		{
			baseColor = albedoTexture->Sample(inResult.paramU, inResult.paramV).RGBToVec3();
		}

		// Default: Lambertian non-metal surface
#if 0
		baseColor = vec3(1.0f);
		float roughness = 0.4f;
		float metallic = 1.0f;
#else
		float roughness = 1.0f;
		float metallic = 0.0f;
#endif
		vec3 N = inResult.n;

		// #todo-pbr: Barely seeing specular highlight without direct sampling.
		// Needs importance sampling; scatter more rays toward specular lobe.
		vec3 Wi = normalize(RandomInHemisphere(N)); // L
		if (Random() < 0.05f) {
			Wi = reflect(inRay.d, N);
		}
		vec3 Wo = -inRay.d; // V
		vec3 H = normalize(Wo + Wi);

		// Early exit
		if (dot(Wo, N) < 0.0f) {
			return false;
		}

#if SUPPORT_PBR_MATERIAL
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
		F0 = mix(F0, min(baseColor, vec3(1.0f)), metallic);

		vec3 F = BRDF::FresnelSchlick(dot(H, Wo), F0);
		float G = BRDF::GeometrySmith(N, Wi, Wo, roughness);
		float NDF = BRDF::DistributionGGX(N, H, roughness);

		vec3 kS = F;
		vec3 kD = 1.0f - kS;
		vec3 diffuse = baseColor * (1.0f - metallic);
		vec3 specular = (F * G * NDF) / (4.0f * dot(N, Wi) * dot(N, Wo) + 0.001f);

		outScattered = ray(inResult.p, Wi, inRay.t);
		// #todo-pbr: Should I multiply cosine weight?
		outAttenuation = (kD * diffuse + kS * specular) * std::max(0.0f, dot(N, Wi));
		//outAttenuation = (kD * diffuse + kS * specular);
		return true;
#else // SUPPORT_PBR_MATERIAL
		outScattered = ray(inResult.p, Wi, inRay.t);
		outAttenuation = baseColor;
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

