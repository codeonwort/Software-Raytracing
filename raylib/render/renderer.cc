#include "renderer.h"
#include "render/image.h"
#include "render/camera.h"
#include "render/material.h"
#include "core/random.h"
#include "core/platform.h"
#include "core/thread_pool.h"
#include "core/stat.h"
#include "core/logger.h"
#include "core/assertion.h"
#include "geom/ray.h"
#include "geom/hit.h"
#include "geom/scene.h"
#include "geom/transform.h"

#include <algorithm>
#include <thread>
#include <vector>

// Determines the number of pixels processed by each task.
#define WORKGROUP_SIZE_X 8
#define WORKGROUP_SIZE_Y 8

// For easy debugging
#define SINGLE_THREADED_RENDERING 0

// oidn is not Windows-only but I'm downloading Windows pre-built binaries.
#if PLATFORM_WINDOWS
	#define INTEL_DENOISER_INTEGRATED 1
	#include <oidn.h>
	#pragma comment(lib, "OpenImageDenoise.lib")
	//#pragma comment(lib, "tbb.lib")
#endif

struct WorkCell {
	int32 x;
	int32 y;
	int32 width;
	int32 height;

	Image2D* image;
	const Camera* camera;
	const Scene* world;
	RendererSettings rendererSettings;
};

// NOTE: Minimize this.
struct RayPayload {
	int32 maxRecursion;
	float rayTMin;
};

bool Renderer::IsDenoiserSupported()
{
#if INTEL_DENOISER_INTEGRATED
	return true;
#else
	return false;
#endif
}

vec3 TraceSceneDebugMode(
	const ray& pathRay,
	const Scene* world,
	const RayPayload& settings,
	ERenderMode debugMode)
{
	vec3 debugValue = vec3(0.0f);
	HitResult hitResult;
	if (world->GetAccelStruct()->Hit(pathRay, settings.rayTMin, FLOAT_MAX, hitResult))
	{
		if (debugMode == ERenderMode::RAYLIB_RENDERMODE_Albedo)
		{
			debugValue = hitResult.material->GetAlbedo(hitResult.paramU, hitResult.paramV);
		}
		else if (debugMode == ERenderMode::RAYLIB_RENDERMODE_SurfaceNormal)
		{
			debugValue = vec3(0.5f) + 0.5f * hitResult.n;
		}
		else if (debugMode == ERenderMode::RAYLIB_RENDERMODE_MicrosurfaceNormal)
		{
			vec3 N = hitResult.material->GetMicrosurfaceNormal(hitResult);
			N = hitResult.LocalToWorld(N);
			debugValue = 0.5f + 0.5f * N;
		}
		else if (debugMode == ERenderMode::RAYLIB_RENDERMODE_Texcoord)
		{
			debugValue = vec3(hitResult.paramU, hitResult.paramV, 0.0f);
		}
		else if (debugMode == ERenderMode::RAYLIB_RENDERMODE_Emission)
		{
			debugValue = hitResult.material->Emitted(hitResult, pathRay.d);
		}
		else if (debugMode == ERenderMode::RAYLIB_RENDERMODE_Reflectance)
		{
			ray dummy; float dummy2;
			debugValue = vec3(1.0f, 0.75f, 0.8f);
			hitResult.material->Scatter(pathRay, hitResult, debugValue, dummy, dummy2);
		}
	}
	return debugValue;
}

// Run path tracing to find incoming radiance.
vec3 TraceScene(
	const ray& pathRay,
	const Scene* world,
	int depth,
	const RayPayload& settings)
{
	if (depth >= settings.maxRecursion)
	{
		return vec3(0.0f);
	}

	// #todo-pbr: Direct sampling of light sources + multiple importance sampling
	// #todo-pbr: Still not sure if I did importance sampling right. Verify again.

	HitResult hitResult;
	if (world->GetAccelStruct()->Hit(pathRay, settings.rayTMin, FLOAT_MAX, hitResult))
	{
		hitResult.BuildOrthonormalBasis();

		vec3 radiance(0.0f);

		// Recursively trace light scattering.
		vec3 reflectance;
		ray scatteredRay;
		float pdf;
		if (hitResult.material->Scatter(pathRay, hitResult, reflectance, scatteredRay, pdf))
		{
			if (pdf > 0.0f)
			{
				vec3 Li = TraceScene(scatteredRay, world, depth + 1, settings);
				float scatteringPdf = hitResult.material->ScatteringPdf(hitResult, -pathRay.d, scatteredRay.d);
				radiance += reflectance * Li * scatteringPdf / pdf;
			}
		}

		// Emission from the surface itself.
		// This is irrelevant to incoming radiances for the surface.
		radiance += hitResult.material->Emitted(hitResult, pathRay.d);
		
		return radiance;
	}

	// If nothing hit, get incoming radiance from sky atmosphere and Sun.
	vec3 missResult(0.0f);
	// Distant lighting: Sky
	ImageHandle skyLightHandle = world->GetSkyPanorama();
	if (skyLightHandle != NULL)
	{
		vec3 dir = pathRay.d;
		dir.Normalize();

		// #todo-wip: Control sky image rotation in Scene.
		Rotator rot;
		rot.yaw = 90.0f;
		vec3 D = rot.rotate(dir);
		//const vec3& D = dir;

		// #todo-wip: Is there a case uv goes to inf or nan?
		float u = std::atan2(D.z, D.x), v = std::asin(D.y);
		u *= 0.1591f; v *= 0.3183f; // inverse atan
		u += 0.5f; v += 0.5f;

		Image2D* skyLight = (Image2D*)skyLightHandle;
		int32 x = (int32)(u * (skyLight->GetWidth() - 1));
		int32 y = (int32)(v * (skyLight->GetHeight() - 1));

		missResult += skyLight->GetPixel(x, y).RGBToVec3();
	}
	else
	{
		// #todo-wip: Fallback sky
		//float t = 0.5f * (dir.y + 1.0f);
		//return ((1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f));
	}
	// Distant lighting: Sun
	vec3 sunDir, sunIlluminance;
	world->GetSun(sunIlluminance, sunDir);
	if (sunIlluminance != vec3(0.0f))
	{
		ray rayToSun(pathRay.o, -sunDir, pathRay.t);
		if (!world->GetAccelStruct()->Hit(rayToSun, settings.rayTMin, FLOAT_MAX, hitResult))
		{
			missResult += sunIlluminance;
		}
	}
	return missResult;
}
// Initial ray is a camera ray.
vec3 TraceScene(
	const ray& cameraRay,
	const Scene* world,
	const RayPayload& settings)
{
	return TraceScene(cameraRay, world, 0, settings);
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
	if (cell->rendererSettings.renderMode == ERenderMode::RAYLIB_RENDERMODE_Default) {
		const int32 SPP = std::max(1, cell->rendererSettings.samplesPerPixel);
		RayPayload rtSettings{
			cell->rendererSettings.maxPathLength,
			cell->rendererSettings.rayTMin,
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
					ray cameraRay = cell->camera->GetCameraRay(u, v);
					vec3 Li = TraceScene(
						cameraRay,
						cell->world,
						rtSettings);
					accum += Li;
				}
				accum /= (float)SPP;
				Pixel px(accum.x, accum.y, accum.z);
				cell->image->SetPixel(x, y, px);
			}
		}
	} else {
		RayPayload rtSettings{
			cell->rendererSettings.maxPathLength,
			cell->rendererSettings.rayTMin,
		};
		for (int32 y = cell->y; y < endY; ++y) {
			for (int32 x = cell->x; x < endX; ++x) {
				float u = (float)x / imageWidth;
				float v = (float)y / imageHeight;
				ray cameraRay = cell->camera->GetCameraRay(u, v);
				vec3 debugValue = TraceSceneDebugMode(
					cameraRay,
					cell->world,
					rtSettings,
					(ERenderMode)cell->rendererSettings.renderMode);
				Pixel px(debugValue.x, debugValue.y, debugValue.z);
				cell->image->SetPixel(x, y, px);
			}
		}
	}
}

void Renderer::RenderScene(
	const RendererSettings* settingsPtr,
	const Scene* world,
	const Camera* camera,
	Image2D* outImage)
{
	CHECK(settingsPtr != nullptr && world != nullptr && camera != nullptr && outImage != nullptr);
	CHECK(world->GetAccelStruct() != nullptr);

#if SINGLE_THREADED_RENDERING
	const uint32 numCores = 1;
	LOG("CAUTION: Rendering is forced to be single threaded - search for 'SINGLE_THREADED_RENDERING'");
#else
	const uint32 numCores = std::max((uint32)1, (uint32)std::thread::hardware_concurrency());
	//LOG("Number of logical cores: %u", numCores);
#endif

	const RendererSettings& settings = *settingsPtr;

	if (settings.viewportWidth != outImage->GetWidth()
		|| settings.viewportHeight != outImage->GetHeight())
	{
		outImage->Reallocate(settings.viewportWidth, settings.viewportHeight);
	}

	const int32 imageWidth = outImage->GetWidth();
	const int32 imageHeight = outImage->GetHeight();
	const int32 spp = settings.samplesPerPixel;

	ThreadPool tp;
	tp.Initialize(numCores);

	std::vector<WorkCell> workCells;
	for (int32 x = 0; x < imageWidth; x += WORKGROUP_SIZE_X) {
		for (int32 y = 0; y < imageHeight; y += WORKGROUP_SIZE_Y) {
			WorkCell cell;
			cell.x = x;
			cell.y = y;
			cell.width = std::min(WORKGROUP_SIZE_X, imageWidth - x);
			cell.height = std::min(WORKGROUP_SIZE_Y, imageHeight - y);
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

	//LOG("number of work items: %d", (int32)workCells.size());

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
					LOG("%d percent complete...", (int32)(progress * 100));
					milestoneIx += 1;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}

bool Renderer::DenoiseScene(
	Image2D* mainImage,
	bool bMainImageHDR,
	Image2D* albedoImage,
	Image2D* normalImage,
	Image2D* outDenoisedImage)
{
	CHECKF(mainImage != nullptr, "mainImage should not be null");
	CHECKF(outDenoisedImage != nullptr, "outDenoisedImage should not be null");
	if (!IsDenoiserSupported())
	{
		return false;
	}

#if INTEL_DENOISER_INTEGRATED
	const size_t viewportWidth = mainImage->GetWidth();
	const size_t viewportHeight = mainImage->GetHeight();

	auto CHECK_EXTENT = [&](Image2D* img) { CHECK(img->GetWidth() == viewportWidth && img->GetHeight() == viewportHeight); };

	// Dump as float array to pass to oidn
	std::vector<float> noisyInputBlob, albedoBlob, worldNormalBlob;
	mainImage->DumpFloatRGBs(noisyInputBlob);
	if (albedoImage != nullptr)
	{
		CHECK_EXTENT(albedoImage);
		albedoImage->DumpFloatRGBs(albedoBlob);
	}
	if (normalImage != nullptr)
	{
		CHECK_EXTENT(normalImage);
		normalImage->DumpFloatRGBs(worldNormalBlob);
	}

	OIDNDevice oidnDevice = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);
	oidnCommitDevice(oidnDevice);

	std::vector<float> denoisedOutputBlob(noisyInputBlob.size(), 0.0f);

	OIDNFilter oidnFilter = oidnNewFilter(oidnDevice, "RT");
	oidnSetSharedFilterImage(oidnFilter, "color", noisyInputBlob.data(),
		OIDN_FORMAT_FLOAT3, viewportWidth, viewportHeight, 0, 0, 0); // noisy input
	if (albedoImage != nullptr)
	{
		oidnSetSharedFilterImage(oidnFilter, "albedo", albedoBlob.data(),
			OIDN_FORMAT_FLOAT3, viewportWidth, viewportHeight, 0, 0, 0); // albedo input
	}
	if (normalImage != nullptr)
	{
		oidnSetSharedFilterImage(oidnFilter, "normal", worldNormalBlob.data(),
			OIDN_FORMAT_FLOAT3, viewportWidth, viewportHeight, 0, 0, 0); // normal input
	}
	oidnSetSharedFilterImage(oidnFilter, "output", denoisedOutputBlob.data(),
		OIDN_FORMAT_FLOAT3, viewportWidth, viewportHeight, 0, 0, 0); // denoised output
	if (bMainImageHDR)
	{
		oidnSetFilter1b(oidnFilter, "hdr", true);
	}
	oidnCommitFilter(oidnFilter);

	oidnExecuteFilter(oidnFilter);

	const char* oidnErr;
	if (oidnGetDeviceError(oidnDevice, &oidnErr) != OIDN_ERROR_NONE)
	{
		LOG("oidn error: %s", oidnErr);
	}

	oidnReleaseFilter(oidnFilter);
	oidnReleaseDevice(oidnDevice);

	outDenoisedImage->Reallocate((uint32)viewportWidth, (uint32)viewportHeight, 0x0);
	size_t k = 0;
	for (uint32_t y = 0; y < viewportHeight; ++y)
	{
		for (uint32_t x = 0; x < viewportWidth; ++x)
		{
			Pixel px(denoisedOutputBlob[k], denoisedOutputBlob[k + 1], denoisedOutputBlob[k + 2]);
			outDenoisedImage->SetPixel(x, y, px);
			k += 3;
		}
	}
#endif // INTEL_DENOISER_INTEGRATED

	return true;
}
