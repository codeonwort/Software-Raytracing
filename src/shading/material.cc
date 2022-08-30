#include "material.h"

// Use Beckmann distribution and shadowing term
#define BECKMANN 1

// -------------------------------

// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
// https://www.gamedev.net/blogs/entry/2261786-microfacet-importance-sampling-for-dummies/
// https://computergraphics.stackexchange.com/questions/4394/path-tracing-the-cook-torrance-brdf

// #todo: Specially for Dielectric
inline float Schlick(float cosine, float ref_idx) {
	float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
	r0 = r0 * r0;
	return r0 + (1.0f - r0) * pow((1.0f - cosine), 5.0f);
}

// -------------------------------
// Metal

bool Metal::Scatter(const ray& inRay, const HitResult& inResult, vec3& outAttenuation, ray& outScattered) const {
	vec3 ud = inRay.d;
	ud.Normalize();
	vec3 reflected = reflect(ud, inResult.n);
	outScattered = ray(inResult.p, reflected + fuzziness * RandomInUnitSphere(), inRay.t);
	outAttenuation = albedo;
	return (dot(outScattered.d, inResult.n) > 0.0f);
}

// -------------------------------
// Dielectric

bool Dielectric::Scatter(const ray& inRay, const HitResult& inResult, vec3& outAttenuation, ray& outScattered) const {
	vec3 outward_normal;
	vec3 reflected = reflect(inRay.d, inResult.n);
	float ni_over_nt;
	outAttenuation = vec3(1.0f, 1.0f, 1.0f);
	vec3 refracted;
	float reflect_prob;
	float cosine;
	if (dot(inRay.d, inResult.n) > 0.0f) {
		outward_normal = -inResult.n;
		ni_over_nt = ref_idx;
		cosine = ref_idx * dot(inRay.d, inResult.n) / inRay.d.Length();
	} else {
		outward_normal = inResult.n;
		ni_over_nt = 1.0f / ref_idx;
		cosine = -dot(inRay.d, inResult.n) / inRay.d.Length();
	}
	if (refract(inRay.d, outward_normal, ni_over_nt, refracted)) {
		reflect_prob = Schlick(cosine, ref_idx);
	} else {
		reflect_prob = 1.0f;
	}
	if (Random() < reflect_prob) {
		outScattered = ray(inResult.p, reflected, inRay.t);
	} else {
		outScattered = ray(inResult.p, refracted, inRay.t);
	}
	return true;
}

// -------------------------------
// PBRMaterial

bool PBRMaterial::Scatter(
	const ray& inRay, const HitResult& inResult,
	vec3& outAttenuation, ray& outScattered) const
{
	vec3 baseColor = albedoFallback;
	float roughness = roughnessFallback;
	float metallic = metallicFallback;

	if (albedoTexture) {
		// #todo-texture: Pre-multiply alpha?
		baseColor = albedoTexture->Sample(inResult.paramU, inResult.paramV).RGBToVec3();
	}
	if (normalmapTexture) {
		vec3 localN = normalmapTexture->Sample(inResult.paramU, inResult.paramV).RGBToVec3();
		// #todo-pbr: Rotate localN around N (normal mapping)
	}
	if (roughnessTexture) {
		roughness = roughnessTexture->Sample(inResult.paramU, inResult.paramV).r;
	}
	if (metallicTexture) {
		metallic = metallicTexture->Sample(inResult.paramU, inResult.paramV).r;
	}

	vec3 N = inResult.n;
	// #todo-pbr: Barely seeing specular highlight without direct sampling.
	// Needs importance sampling; scatter more rays toward specular lobe.
	vec3 Wi = normalize(RandomInHemisphere(N)); // L
	//if (roughness < 1.0f && Random() < 0.05f) {
	//	vec3 R = reflect(inRay.d, N);
	//	//Wi = normalize(mix(R, Wi, roughness));
	//	Wi = R;
	//}
	vec3 Wo = -inRay.d; // V
	vec3 H = normalize(Wo + Wi);

	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, baseColor, metallic);

	vec3 F = BRDF::FresnelSchlick(dot(H, Wo), F0);
#if BECKMANN
	float G = BRDF::GeometrySmith_Beckmann(N, H, Wo, Wi, roughness);
	float NDF = BRDF::DistributionBeckmann(N, H, roughness);
#else
	float G = BRDF::GeometrySmith_SchlickGGX(N, Wi, Wo, roughness);
	float NDF = BRDF::DistributionGGX(N, H, roughness);
#endif

	vec3 kS = F;
	vec3 kD = 1.0f - kS;
	vec3 diffuse = baseColor * (1.0f - metallic);
	vec3 specular = (F * G * NDF) / (4.0f * dot(N, Wi) * dot(N, Wo) + 0.001f);

	outScattered = ray(inResult.p, Wi, inRay.t);
	// #todo-pbr: Should I multiply cosine weight?
	outAttenuation = (kD * diffuse + kS * specular) * abs(dot(N, Wi));
	return true;
}

vec3 PBRMaterial::Emitted(float u, float v, const vec3& inPosition) const {
	if (emissiveTexture) {
		Pixel emissiveSample = emissiveTexture->Sample(u, v);
		return vec3(emissiveSample.r, emissiveSample.g, emissiveSample.b);
	}
	return emissiveFallback;
}
