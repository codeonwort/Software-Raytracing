#pragma once

#include "raylib_types.h"
#include "core/int_types.h"
#include "core/vec3.h"
#include <functional>

class Hitable;
class Camera;
class Image2D;

using FakeSkyLightFunction = std::function<vec3(const vec3& rayDir)>;
using FakeSunLightFunction = std::function<void(vec3& outDir, vec3& outIlluminance)>;

struct RendererSettings {
	// Camera properties
	uint32               viewportWidth;
	uint32               viewportHeight;
	vec3                 cameraLocation;
	vec3                 cameraLookat;

	// Path tracing options
	int32                samplesPerPixel;
	int32                maxPathLength;
	float                rayTMin;

	// Global lighting environment
	FakeSkyLightFunction skyLightFn      = nullptr;
	FakeSunLightFunction sunLightFn      = nullptr;

	// System values
	ERenderMode          renderMode      = ERenderMode::RAYLIB_RENDERMODE_Default;

	inline float getViewportAspectWH() const {
		return (float)viewportWidth / (float)viewportHeight;
	}
};

class Renderer
{
public:
	static bool IsDenoiserSupported();

	RAYLIB_API void RenderScene(
		const RendererSettings& settings,
		const Hitable* world,
		const Camera* camera,
		Image2D* outImage);

	RAYLIB_API bool DenoiseScene(
		Image2D* mainImage,
		bool bMainImageHDR,
		Image2D* albedoImage,
		Image2D* normalImage,
		Image2D*& outDenoisedImage);
};
