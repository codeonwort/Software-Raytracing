#pragma once

#include "raylib_types.h"
#include "core/int_types.h"
#include "core/vec3.h"
#include <functional>

class Hitable;
class Camera;
class Scene;
class Image2D;

struct RendererSettings {
	// Camera properties
	uint32               viewportWidth;
	uint32               viewportHeight;

	// Path tracing options
	int32                samplesPerPixel;
	int32                maxPathLength;
	float                rayTMin;

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

	// #todo-raylib: Remove dllexport
	RAYLIB_API void RenderScene(
		const RendererSettings* settings,
		const Scene* world,
		const Camera* camera,
		Image2D* outImage);

	bool DenoiseScene(
		Image2D* mainImage,
		bool bMainImageHDR,
		Image2D* albedoImage,
		Image2D* normalImage,
		Image2D* outDenoisedImage);
};
