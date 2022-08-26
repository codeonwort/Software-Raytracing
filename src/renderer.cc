#include "renderer.h"
#include "thread_pool.h"
#include "log.h"
#include "image.h"
#include "camera.h"
#include "random.h"
#include "geom/ray.h"
#include "geom/hit.h"
#include "shading/material.h"
#include "util/stat.h"

#include <algorithm>
#include <thread>
#include <vector>

struct WorkCell {
	int32 x;
	int32 y;
	int32 width;
	int32 height;

	Image2D* image;
	const Camera* camera;
	const Hitable* world;
	RendererSettings rendererSettings;
};

// NOTE: Minimize this.
struct RayTracingSettings {
	int32 maxRecursion;
	float rayTMin;
	bool fakeSkyLight;
};

vec3 TraceScene(const ray& r, const Hitable* world, int depth, const RayTracingSettings& settings) {
	HitResult result;
	if (world->Hit(r, settings.rayTMin, FLOAT_MAX, result)) {
		ray scattered;
		vec3 attenuation;
		vec3 emitted = result.material->Emitted(result.paramU, result.paramV, result.p);
		if (depth < settings.maxRecursion && result.material->Scatter(r, result, attenuation, scattered)) {
			return emitted + attenuation * TraceScene(scattered, world, depth + 1, settings);
		} else {
			return emitted;
		}
	}

	if (settings.fakeSkyLight) {
		vec3 dir = r.d;
		dir.Normalize();
		float t = 0.5f * (dir.y + 1.0f);
		return 3.0f * ((1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f));
	}
	return vec3(0.0f, 0.0f, 0.0f);
}
vec3 TraceScene(const ray& r, const Hitable* world, const RayTracingSettings& settings) {
	return TraceScene(r, world, 0, settings);
}

void GenerateCell(const WorkItemParam* param) {
	static thread_local RNG randomsAA(4096 * 8);

	int32 threadID = param->threadID;
	WorkCell* cell = reinterpret_cast<WorkCell*>(param->arg);

	const int32 endY = cell->y + cell->height;
	const int32 endX = cell->x + cell->width;

	const float imageWidth = (float)cell->image->GetWidth();
	const float imageHeight = (float)cell->image->GetHeight();

	// #todo-multithread: Bad utilization of threads; Some cells might take longer than others.
	const int32 SPP = std::max(1, cell->rendererSettings.samplesPerPixel);
	RayTracingSettings rtSettings{
		cell->rendererSettings.maxPathLength,
		cell->rendererSettings.rayTMin,
		cell->rendererSettings.fakeSkyLight
	};
	for (int32 y = cell->y; y < endY; ++y) {
		for (int32 x = cell->x; x < endX; ++x) {
			vec3 accum(0.0f, 0.0f, 0.0f);
			for (int32 s = 0; s < SPP; ++s) {
				float u = (float)x / imageWidth;
				float v = (float)y / imageHeight;
				if (s != 0) {
					u += (randomsAA.Peek() - 0.5f) * 2.0f / imageWidth;
					v += (randomsAA.Peek() - 0.5f) * 2.0f / imageHeight;
				}
				ray r = cell->camera->GetRay(u, v);
				vec3 scene = TraceScene(r, cell->world, rtSettings);
				accum += scene;
			}
			accum /= (float)SPP;
			Pixel px(accum.x, accum.y, accum.z);
			cell->image->SetPixel(x, y, px);
		}
	}
}

void Renderer::RenderScene(
	const RendererSettings& settings,
	const Hitable* world,
	const Camera* camera,
	Image2D* outImage)
{
	const uint32 numCores = std::max((uint32)1, (uint32)std::thread::hardware_concurrency());
	log("number of logical cores: %u", numCores);

	const int32 imageWidth = outImage->GetWidth();
	const int32 imageHeight = outImage->GetHeight();
	const int32 spp = settings.samplesPerPixel;

	ThreadPool tp;
	tp.Initialize(numCores);

	std::vector<WorkCell> workCells;
	constexpr int32 cellWidth = 32;
	constexpr int32 cellHeight = 32;
	for (int32 x = 0; x < imageWidth; x += cellWidth) {
		for (int32 y = 0; y < imageHeight; y += cellHeight) {
			WorkCell cell;
			cell.x = x;
			cell.y = y;
			cell.width = std::min(cellWidth, imageWidth - x);
			cell.height = std::min(cellHeight, imageHeight - y);
			cell.image = outImage;
			cell.camera = camera;
			cell.world = world;
			cell.rendererSettings = settings;
			workCells.emplace_back(cell);
		}
	}
	for (auto i = 0u; i < workCells.size(); ++i) {
		ThreadPoolWork work;
		work.routine = GenerateCell;
		work.arg = &workCells[i];

		tp.AddWork(work);
	}

	log("number of work items: %d", (int32)workCells.size());

	{
		SCOPED_CPU_COUNTER(ThreadPoolWorkTime);

		constexpr bool blockingOperation = false;
		tp.Start(blockingOperation);

		// progress
		int32 milestoneIx = 0;
		std::vector<float> milestones(9);
		for (int32 i = 1; i <= 9; ++i) {
			milestones[i - 1] = (float)i / 10.0f;
		}

		while (true) {
			if (tp.IsDone()) {
				break;
			} else {
				float progress = tp.GetProgress();
				if (milestoneIx < milestones.size() && progress >= milestones[milestoneIx]) {
					log("%d percent complete...", (int32)(progress * 100));
					milestoneIx += 1;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}
