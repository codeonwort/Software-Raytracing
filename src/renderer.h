#pragma once

#include "type.h"
#include "core/vec.h"
#include <functional>

class Hitable;
class Camera;
class Image2D;

using FakeSkyLightFunction = std::function<vec3(const vec3& rayDir)>;
using FakeSunLightFunction = std::function<void(vec3& outDir, vec3& outIlluminance)>;

enum class EDebugMode : uint32 {
	None = 0,
	VertexNormal,
	Texcoord,
	Reflectance,
	ReflectanceFromOneBounce, // To debug mirror reflection
	Emission,
	
	MAX,
};

const char* GetRendererDebugModeString(int32 modeIx);

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
	EDebugMode           debugMode       = EDebugMode::None;
	bool                 bRunDenoiser    = false;

	inline float getViewportAspectWH() const {
		return (float)viewportWidth / (float)viewportHeight;
	}
};

class Renderer
{
	
public:
	void RenderScene(
		const RendererSettings& settings,
		const Hitable* world,
		const Camera* camera,
		Image2D* outImage);

};
