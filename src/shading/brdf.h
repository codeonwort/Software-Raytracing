#pragma once

#include "type.h"

namespace BRDF {

	const float PI = 3.14159265359f;

	// cosTheta = dot(incident_or_exitant_light, half_vector)
	inline vec3 FresnelSchlick(float cosTheta, const vec3& F0) {
		return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
	}

	inline vec3 FresnelSchlickRoughness(float cosTheta, const vec3& F0, float roughness) {
		return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(1.0f - cosTheta, 5.0f);
	}

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

	inline float GeometrySchlickGGX(float NdotV, float roughness) {
		float r = (roughness + 1.0f);
		float k = (r * r) / 8.0f;

		float num = NdotV;
		float denom = NdotV * (1.0f - k) + k;

		return num / denom;
	}

	inline float GeometrySmith(const vec3& N, const vec3& V, const vec3& L, float roughness) {
		float NdotV = abs(dot(N, V));
		float NdotL = abs(dot(N, L));
		float ggx2 = GeometrySchlickGGX(NdotV, roughness);
		float ggx1 = GeometrySchlickGGX(NdotL, roughness);

		return ggx1 * ggx2;
	}
};
