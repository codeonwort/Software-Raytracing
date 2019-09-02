#pragma once

#include "ray.h"
#include "hit.h"
#include "random.h"

class Material
{

public:
	// 1. Produce a scattered ray
	// 2. If scattered, tell how much the ray should be attenuated
	virtual bool Scatter(
		const ray& inRay, const HitResult& inResult,
		vec3& outAttenuation, ray& outScattered) const = 0;

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
		outScattered = ray(inResult.p, target - inResult.p);
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
		vec3& outAttenuation, ray& outScattered) const override
	{
		vec3 ud = inRay.d;
		ud.Normalize();
		vec3 reflected = reflect(ud, inResult.n);
		outScattered = ray(inResult.p, reflected + fuzziness * RandomInUnitSphere());
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
			outScattered = ray(inResult.p, reflected);
		}
		else
		{
			outScattered = ray(inResult.p, refracted);
		}
		return true;
	}

	float ref_idx;

};

