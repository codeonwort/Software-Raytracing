#pragma once

#include "type.h"

namespace BRDF {

	const float PI = 3.14159265359f;

	// -------------------------------
	// Fresnel term

	// cosTheta = dot(incident_or_exitant_light, half_vector)
	inline vec3 FresnelSchlick(float cosTheta, const vec3& F0) {
		return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
	}

	inline vec3 FresnelSchlickRoughness(float cosTheta, const vec3& F0, float roughness) {
		return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
	}

	// -------------------------------
	// Distribution term

	inline float DistributionGGX(const vec3& N, const vec3& H, float roughness) {
		float a = roughness * roughness;
		float a2 = a * a;
		float NdotH = dot(N, H);
		float NdotH2 = NdotH * NdotH;

		float num = a2;
		float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
		denom = PI * denom * denom;

		return num / denom;
	}

	inline float DistributionBeckmann(const vec3& N, const vec3& H, float roughness) {
		float cosH = dot(N, H);
		float cosH2 = cosH * cosH;
		float thetaH = acosf(cosH);
		float tanH = tanf(thetaH);
		float rr = roughness * roughness;
		float num = (cosH > 0.0f ? 1.0f : 0.0f) * expf(-tanH * tanH / rr);
		float denom = BRDF::PI * rr * cosH2 * cosH2;
		return num / denom;
	}

	// -------------------------------
	// Geometry term

	inline float GeometrySchlickGGX(float NdotV, float roughness) {
		float r = (roughness + 1.0f);
		float k = (r * r) / 8.0f;

		float num = NdotV;
		float denom = NdotV * (1.0f - k) + k;

		return num / denom;
	}

	// N: macrosurface normal
	inline float GeometryBeckmann(
		const vec3& N, const vec3& H,
		const vec3& V, float roughness)
	{
		float thetaV = acosf(dot(N, V));
		float tanThetaV = tanf(thetaV);
		float a = 1.0f / (roughness * tanThetaV);
		float aa = a * a;

		if (dot(V, H) / dot(V, N) <= 0.0f) {
			return 0.0f;
		}

		if (a < 1.6f) {
			float num = 3.535f * a + 2.181f * aa;
			float denom = 1.0f + 2.276f * a + 2.577f * aa;
			return num / denom;
		}
		return 1.0f;
	}

	inline float GeometrySmith_SchlickGGX(
		const vec3& N, const vec3& V,
		const vec3& L, float roughness)
	{
		float NdotV = abs(dot(N, V));
		float NdotL = abs(dot(N, L));
		float ggx2 = GeometrySchlickGGX(NdotV, roughness);
		float ggx1 = GeometrySchlickGGX(NdotL, roughness);

		return ggx1 * ggx2;
	}

	inline float GeometrySmith_Beckmann(
		const vec3& N, const vec3& H,
		const vec3& V, const vec3& L,
		float roughness)
	{
		float ggx2 = GeometryBeckmann(N, H, V, roughness);
		float ggx1 = GeometryBeckmann(N, H, L, roughness);
		return ggx1 * ggx2;
	}

};
