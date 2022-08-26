#pragma once

#include "type.h"

class Hitable;
class Camera;
class Image2D;

struct RendererSettings {
	int32 samplesPerPixel;
	int32 maxPathLength;
	float rayTMin;

	// #todo: Support sky cubemap
	bool fakeSkyLight;
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
