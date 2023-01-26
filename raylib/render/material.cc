#include "material.h"

// #todo: Include in renderer settings?
#define FURNACE_TEST 0
#define CUTOUT_ALPHA 0.5f

// -------------------------------

// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.pdf
// https://www.gamedev.net/blogs/entry/2261786-microfacet-importance-sampling-for-dummies/
// https://computergraphics.stackexchange.com/questions/4394/path-tracing-the-cook-torrance-brdf

// -------------------------------
// From https://github.com/mmp/pbrt-v3

inline float Clamp(float val, float minVal, float maxVal) {
	return std::max(minVal, std::min(maxVal, val));
}
inline float ErfInv(float x) {
	float w, p;
	x = Clamp(x, -.99999f, .99999f);
	w = -std::log((1 - x) * (1 + x));
	if (w < 5) {
		w = w - 2.5f;
		p = 2.81022636e-08f;
		p = 3.43273939e-07f + p * w;
		p = -3.5233877e-06f + p * w;
		p = -4.39150654e-06f + p * w;
		p = 0.00021858087f + p * w;
		p = -0.00125372503f + p * w;
		p = -0.00417768164f + p * w;
		p = 0.246640727f + p * w;
		p = 1.50140941f + p * w;
	} else {
		w = std::sqrt(w) - 3;
		p = -0.000200214257f;
		p = 0.000100950558f + p * w;
		p = 0.00134934322f + p * w;
		p = -0.00367342844f + p * w;
		p = 0.00573950773f + p * w;
		p = -0.0076224613f + p * w;
		p = 0.00943887047f + p * w;
		p = 1.00167406f + p * w;
		p = 2.83297682f + p * w;
	}
	return p * x;
}
inline float Erf(float x) {
	// constants
	float a1 = 0.254829592f;
	float a2 = -0.284496736f;
	float a3 = 1.421413741f;
	float a4 = -1.453152027f;
	float a5 = 1.061405429f;
	float p = 0.3275911f;

	// Save the sign of x
	int sign = 1;
	if (x < 0) sign = -1;
	x = std::abs(x);

	// A&S formula 7.1.26
	float t = 1 / (1 + p * x);
	float y =
		1 -
		(((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * std::exp(-x * x);

	return sign * y;
}
inline float CosTheta(const vec3& w) { return w.z; }
inline float Cos2Theta(const vec3& w) { return w.z * w.z; }
inline float AbsCosTheta(const vec3& w) { return std::abs(w.z); }
inline float Sin2Theta(const vec3& w) { return std::max(0.0f, 1.0f - Cos2Theta(w)); }
inline float SinTheta(const vec3& w) { return std::sqrt(Sin2Theta(w)); }
inline float CosPhi(const vec3& w) {
	float sinTheta = SinTheta(w);
	return (sinTheta == 0) ? 1 : Clamp(w.x / sinTheta, -1, 1);
}
inline float SinPhi(const vec3& w) {
	float sinTheta = SinTheta(w);
	return (sinTheta == 0) ? 0 : Clamp(w.y / sinTheta, -1, 1);
}
static void BeckmannSample11(
	float cosThetaI, float U1, float U2,
	float* slope_x, float* slope_y)
{
	const float Pi = BRDF::PI;

	/* Special case (normal incidence) */
	if (cosThetaI > .9999) {
		float r = std::sqrt(-std::log(1.0f - U1));
		float sinPhi = std::sin(2 * Pi * U2);
		float cosPhi = std::cos(2 * Pi * U2);
		*slope_x = r * cosPhi;
		*slope_y = r * sinPhi;
		return;
	}

	/* The original inversion routine from the paper contained
	   discontinuities, which causes issues for QMC integration
	   and techniques like Kelemen-style MLT. The following code
	   performs a numerical inversion with better behavior */
	float sinThetaI =
		std::sqrt(std::max((float)0, (float)1 - cosThetaI * cosThetaI));
	float tanThetaI = sinThetaI / cosThetaI;
	float cotThetaI = 1 / tanThetaI;

	/* Search interval -- everything is parameterized
	   in the Erf() domain */
	float a = -1, c = Erf(cotThetaI);
	float sample_x = std::max(U1, (float)1e-6f);

	/* Start with a good initial guess */
	// float b = (1-sample_x) * a + sample_x * c;

	/* We can do better (inverse of an approximation computed in
	 * Mathematica) */
	float thetaI = std::acos(cosThetaI);
	float fit = 1 + thetaI * (-0.876f + thetaI * (0.4265f - 0.0594f * thetaI));
	float b = c - (1 + c) * std::pow(1 - sample_x, fit);

	/* Normalization factor for the CDF */
	static const float SQRT_PI_INV = 1.f / std::sqrt(Pi);
	float normalization =
		1 /
		(1 + c + SQRT_PI_INV * tanThetaI * std::exp(-cotThetaI * cotThetaI));

	int it = 0;
	while (++it < 10) {
		/* Bisection criterion -- the oddly-looking
		   Boolean expression are intentional to check
		   for NaNs at little additional cost */
		if (!(b >= a && b <= c)) b = 0.5f * (a + c);

		/* Evaluate the CDF and its derivative
		   (i.e. the density function) */
		float invErf = ErfInv(b);
		float value =
			normalization *
			(1 + b + SQRT_PI_INV * tanThetaI * std::exp(-invErf * invErf)) -
			sample_x;
		float derivative = normalization * (1 - invErf * tanThetaI);

		if (std::abs(value) < 1e-5f) break;

		/* Update bisection intervals */
		if (value > 0)
			c = b;
		else
			a = b;

		b -= value / derivative;
	}

	/* Now convert back into a slope value */
	*slope_x = ErfInv(b);

	/* Simulate Y component */
	*slope_y = ErfInv(2.0f * std::max(U2, (float)1e-6f) - 1.0f);

	CHECK(!std::isinf(*slope_x));
	CHECK(!std::isnan(*slope_x));
	CHECK(!std::isinf(*slope_y));
	CHECK(!std::isnan(*slope_y));
}
static vec3 BeckmannSample(
	const vec3& wi,
	float alpha_x, float alpha_y,
	float U1, float U2)
{
	// 1. stretch wi
	vec3 wiStretched =
		normalize(vec3(alpha_x * wi.x, alpha_y * wi.y, wi.z));

	// 2. simulate P22_{wi}(x_slope, y_slope, 1, 1)
	float slope_x, slope_y;
	BeckmannSample11(CosTheta(wiStretched), U1, U2, &slope_x, &slope_y);

	// 3. rotate
	float tmp = CosPhi(wiStretched) * slope_x - SinPhi(wiStretched) * slope_y;
	slope_y = SinPhi(wiStretched) * slope_x + CosPhi(wiStretched) * slope_y;
	slope_x = tmp;

	// 4. unstretch
	slope_x = alpha_x * slope_x;
	slope_y = alpha_y * slope_y;

	// 5. compute normal
	return normalize(vec3(-slope_x, -slope_y, 1.f));
}

// -------------------------------
// Lambertian

bool Lambertian::Scatter(
	const ray& pathRay,
	const HitResult& hitResult,
	vec3& outReflectance,
	ray& outScatteredRay,
	float& outPdf) const
{
	vec3 N = hitResult.n;
	vec3 Wi = normalize(RandomInHemisphere(N));

	outScatteredRay = ray(hitResult.p, Wi, pathRay.t);
	outReflectance = albedo;
	outPdf = absDot(N, Wi) / BRDF::PI;

	return true;
}

float Lambertian::ScatteringPdf(
	const HitResult& hitResult,
	const vec3& Wo,
	const vec3& Wi) const
{
	float cosTheta = std::max(0.0f, dot(hitResult.n, Wi));
	return cosTheta / BRDF::PI;
}


// -------------------------------
// Metal

bool Metal::Scatter(
	const ray& pathRay,
	const HitResult& hitResult,
	vec3& outReflectance,
	ray& outScatteredRay,
	float& outPdf) const
{
	vec3 ud = pathRay.d;
	ud.Normalize();
	vec3 reflected = reflect(ud, hitResult.n);
	outScatteredRay = ray(hitResult.p, reflected + fuzziness * RandomInUnitSphere(), pathRay.t);
	outReflectance = albedo;
	outPdf = 1.0f;
	return (dot(outScatteredRay.d, hitResult.n) > 0.0f);
}

// -------------------------------
// Dielectric

inline float Schlick(float cosine, float ref_idx) {
	float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
	r0 = r0 * r0;
	return r0 + (1.0f - r0) * pow((1.0f - cosine), 5.0f);
}

bool Dielectric::Scatter(
	const ray& pathRay,
	const HitResult& hitResult,
	vec3& outReflectance,
	ray& outScatteredRay,
	float& outPdf) const
{
	vec3 outward_normal;
	vec3 reflected = reflect(pathRay.d, hitResult.n);
	float ni_over_nt;
	outReflectance = transmissionFilter;
	vec3 refracted;
	float reflect_prob;
	float cosine;
	if (dot(pathRay.d, hitResult.n) > 0.0f) {
		outward_normal = -hitResult.n;
		ni_over_nt = ref_idx;
		cosine = ref_idx * dot(pathRay.d, hitResult.n) / pathRay.d.Length();
	} else {
		outward_normal = hitResult.n;
		ni_over_nt = 1.0f / ref_idx;
		cosine = -dot(pathRay.d, hitResult.n) / pathRay.d.Length();
	}
	if (refract(pathRay.d, outward_normal, ni_over_nt, refracted)) {
		reflect_prob = Schlick(cosine, ref_idx);
	} else {
		reflect_prob = 1.0f;
	}
	if (Random() < reflect_prob) {
		outScatteredRay = ray(hitResult.p, reflected, pathRay.t);
	} else {
		outScatteredRay = ray(hitResult.p, refracted, pathRay.t);
	}
	outPdf = 1.0f;
	return true;
}

// -------------------------------
// MicrofacetMaterial

bool MicrofacetMaterial::Scatter(
	const ray& pathRay, const HitResult& hitResult,
	vec3& outReflectance, ray& outScatteredRay,
	float& outPdf) const
{
	vec3 baseColor = GetAlbedo(hitResult.paramU, hitResult.paramV);
	float roughness = roughnessFallback;
	float metallic = metallicFallback;

	if (roughnessTexture) {
		roughness = roughnessTexture->Sample(hitResult.paramU, hitResult.paramV).r;
	}
	if (metallicTexture) {
		metallic = metallicTexture->Sample(hitResult.paramU, hitResult.paramV).r;
	}

#if FURNACE_TEST
	baseColor = vec3(0.18f);
	roughness = 1.0f;
	metallic = 0.0f;
#endif

	// Do calculation in local space
	vec3 N = GetMicrosurfaceNormal(hitResult);
	vec3 Wo = hitResult.WorldToLocal(-pathRay.d);
	vec3 Wh = Sample_wh(Wo, roughness);
	vec3 Wi = reflect(-Wo, Wh);
	float NdotWi = absDot(N, Wi);

	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, baseColor, metallic);

	vec3 F = BRDF::FresnelSchlick(absDot(Wh, Wo), F0);
	float G = BRDF::GeometrySmith_Beckmann(N, Wh, Wo, Wi, roughness);
	float NDF = BRDF::DistributionBeckmann(N, Wh, roughness);

	vec3 kS = F;
	vec3 kD = 1.0f - kS;
	vec3 diffuse = baseColor * (1.0f - metallic);
	vec3 specular = (F * G * NDF) / (4.0f * NdotWi * absDot(N, Wo) + 0.001f);

	// Transform to world space
	Wi = hitResult.LocalToWorld(Wi);

	// #todo-wip: [FATAL] Not energy conserving?
	// Especially in DabrovicSponza all goes white.
	outScatteredRay = ray(hitResult.p, Wi, pathRay.t);
	outReflectance = (kD * diffuse + kS * specular) * NdotWi;
	outPdf = ScatteringPdf(hitResult, -pathRay.d, outScatteredRay.d) / (4.0f * dot(Wo, Wh));
	return true;
}

vec3 MicrofacetMaterial::Emitted(const HitResult& hitResult, const vec3& Wo) const {
	vec3 emit = emissiveFallback;
	if (emissiveTexture) {
		Pixel emissiveSample = emissiveTexture->Sample(hitResult.paramU, hitResult.paramU);
		emit = (emissiveSample.r, emissiveSample.g, emissiveSample.b);
	}

	return emit;
}

float MicrofacetMaterial::ScatteringPdf(
	const HitResult& hitResult,
	const vec3& Wo_world,
	const vec3& Wi_world) const
{
	vec3 wo = hitResult.WorldToLocal(Wo_world);
	vec3 wi = hitResult.WorldToLocal(Wi_world);
	vec3 wh = normalize(wo + wi);
	if (wh.z < 0.0f) {
		wh.z = -wh.z;
	}
	vec3 n = GetMicrosurfaceNormal(hitResult);

	float roughness = roughnessFallback;
	if (roughnessTexture) {
		roughness = roughnessTexture->Sample(hitResult.paramU, hitResult.paramV).r;
	}

#if FURNACE_TEST
	roughness = 1.0f;
#endif

	float D = BRDF::DistributionBeckmann(n, wh, roughness);
	return D * absDot(wh, n);
}

bool MicrofacetMaterial::IsMirrorLike(float paramU, float paramV) const
{
	float roughness = roughnessFallback;
	if (roughnessTexture) {
		roughness = roughnessTexture->Sample(paramU, paramV).r;
	}
	return roughness < 0.1f;
}

vec3 MicrofacetMaterial::GetAlbedo(float paramU, float paramV) const
{
	vec3 albedo = albedoFallback;
	if (albedoTexture) {
		Pixel albedoPixel = albedoTexture->Sample(paramU, paramV);
		albedo = albedoPixel.RGBToVec3() * albedoPixel.a;
	}
	return albedo;
}

bool MicrofacetMaterial::AlphaTest(float texcoordU, float texcoordV) const
{
	if (albedoTexture != nullptr) {
		Pixel albedoPixel = albedoTexture->Sample(texcoordU, texcoordV);
		return albedoPixel.a >= CUTOUT_ALPHA;
	}
	return true;
}

vec3 MicrofacetMaterial::GetMicrosurfaceNormal(const HitResult& hitResult) const
{
	if (normalmapTexture)
	{
		vec3 N = normalmapTexture->Sample(hitResult.paramU, hitResult.paramV).RGBToVec3();
		N = normalize(2.0f * N - 1.0f);
		return N;
	}
	return vec3(0.0f, 0.0f, 1.0f);
}

vec3 MicrofacetMaterial::Sample_wh(const vec3& wo, float alpha) const
{
	// https://pbr-book.org/3ed-2018/Light_Transport_I_Surface_Reflection/Sampling_Reflection_Functions#sec:microfacet-sample

	float u0 = Random();
	float u1 = Random();

	// Sample from the distribution of visible microfacets from a given wo.
	vec3 wh;
	bool bFlip = wo.z < 0.0f;
	wh = BeckmannSample(bFlip ? -wo : wo, alpha, alpha, u0, u1);
	if (bFlip) wh = -wh;

	return wh;
}
