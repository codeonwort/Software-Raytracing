#include "material.h"

// #todo: Include in renderer settings?
#define FURNACE_TEST 0
#define CUTOUT_ALPHA 0.5f

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
// PBRMaterial

// #todo-pbr: Is my specular lobe right?
// roughness = 0.0 should act like a mirror
// but it seems all rays are scattered to the surface normal in that case...
bool MicrofacetMaterial::Scatter(
	const ray& pathRay, const HitResult& hitResult,
	vec3& outReflectance, ray& outScatteredRay,
	float& outPdf) const
{
	vec3 baseColor = albedoFallback;
	float roughness = roughnessFallback;
	float metallic = metallicFallback;

	if (albedoTexture) {
		Pixel albedoPixel = albedoTexture->Sample(hitResult.paramU, hitResult.paramV);
		baseColor = albedoPixel.RGBToVec3() * albedoPixel.a;
	}
	if (normalmapTexture) {
		vec3 localN = normalmapTexture->Sample(hitResult.paramU, hitResult.paramV).RGBToVec3();
		// #todo-pbr: Rotate localN around N (normal mapping)
	}
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

	vec3 N = vec3(0.0f, 0.0f, 1.0f);
	vec3 Wo = hitResult.WorldToLocal(-pathRay.d);
	vec3 Wi;
	
	float u1 = Random();
	float u2 = Random();
	float theta = std::atan(std::sqrt(-roughness * roughness * std::log(1.0f - u1)));
	float phi = 2.0f * BRDF::PI * u2;
	float xi = std::cos(phi) * std::sin(theta);
	float yi = std::sin(phi) * std::sin(theta);
	float zi = std::cos(theta);
	Wi = vec3(xi, yi, zi);
	if (Wo.z < 0.0f) {
		Wi.z = -Wi.z;
	}
	
	vec3 H = normalize(Wo + Wi);

	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, baseColor, metallic);

	vec3 F = BRDF::FresnelSchlick(absDot(H, Wo), F0);
	float G = BRDF::GeometrySmith_Beckmann(N, H, Wo, Wi, roughness);
	float NDF = BRDF::DistributionBeckmann(N, H, roughness);

	vec3 kS = F;
	vec3 kD = 1.0f - kS;
	vec3 diffuse = baseColor * (1.0f - metallic);
	vec3 specular = (F * G * NDF) / (4.0f * absDot(N, Wi) * absDot(N, Wo) + 0.001f);

	float NdotWi = absDot(N, Wi);

	// Transform to world space
	Wi = hitResult.LocalToWorld(Wi);

	outScatteredRay = ray(hitResult.p, Wi, pathRay.t);
	outReflectance = (kD * diffuse + kS * specular) * NdotWi;
	outPdf = ScatteringPdf(hitResult, -pathRay.d, outScatteredRay.d) / (4.0f * dot(Wo, H));
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
	vec3 n(0.0f, 0.0f, 1.0f);

	float roughness = roughnessFallback;
	if (roughnessTexture) {
		roughness = roughnessTexture->Sample(hitResult.paramU, hitResult.paramV).r;
	}

#if FURNACE_TEST
	roughness = 1.0f;
#endif

	float D = BRDF::DistributionBeckmann(n, wh, roughness);
	return D * absDot(wh, hitResult.n);
}

bool MicrofacetMaterial::AlphaTest(float texcoordU, float texcoordV)
{
	if (albedoTexture != nullptr) {
		Pixel albedoPixel = albedoTexture->Sample(texcoordU, texcoordV);
		return albedoPixel.a >= CUTOUT_ALPHA;
	}
	return true;
}
