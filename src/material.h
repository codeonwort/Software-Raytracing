#pragma once

#include "random.h"
#include "geom/ray.h"
#include "geom/hit.h"

#include "image.h"
#include "texture.h"

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
		vec3 target = inResult.p + inResult.n + RandomInUnitSphere();
		outScattered = ray(inResult.p, target - inResult.p, inRay.t);
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
		vec3 normal(0.0f, 0.0f, 1.0f);
		vec3 roughness(1.0f, 1.0f, 1.0f);
		vec3 metallic(0.0f, 0.0f, 0.0f);

		// #todo-texture: Pre-multiply alpha?
		if (albedoTexture)
		{
			Pixel albedoSample = albedoTexture->Sample(inResult.paramU, inResult.paramV);
			albedo = vec3(albedoSample.r, albedoSample.g, albedoSample.b);
		}

		// #todo-pbr: Utilize them
		if (normalmapTexture)
		{
			Pixel normalSample = normalmapTexture->Sample(inResult.paramU, inResult.paramV);
		}
		if (roughnessTexture)
		{
			Pixel roughnessSample = roughnessTexture->Sample(inResult.paramU, inResult.paramV);
		}
		if (metallicTexture)
		{
			Pixel metallicSample = metallicTexture->Sample(inResult.paramU, inResult.paramV);
		}

		vec3 target = inResult.p + inResult.n + RandomInUnitSphere();
		outScattered = ray(inResult.p, target - inResult.p, inRay.t);
		outAttenuation = albedo;
		return true;
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

