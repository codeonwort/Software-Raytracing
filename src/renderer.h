#pragma once

#include "type.h"
#include "core/vec.h"
#include <functional>

class Hitable;
class Camera;
class Image2D;

using FakeSkyLightFunction = std::function<vec3(const vec3& rayDir)>;

enum class EDebugMode {
	None,
	VertexNormal,
	Texcoord,
	Reflectance,
};

struct RendererSettings {
	int32 samplesPerPixel;
	int32 maxPathLength;
	float rayTMin;

	FakeSkyLightFunction skyLightFn = nullptr;

	EDebugMode debugMode = EDebugMode::None;
};

class Renderer {
	
public:
	void RenderScene(
		const RendererSettings& settings,
		const Hitable* world,
		const Camera* camera,
		Image2D* outImage);

private:
	//

};
