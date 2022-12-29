#pragma once

#include "raylib_types.h"
#include "core/int_types.h"
#include "core/vec3.h"

class Hitable;
class Camera;
class Scene;
class Image2D;

class Renderer
{
public:
	static bool IsDenoiserSupported();

	void RenderScene(
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
