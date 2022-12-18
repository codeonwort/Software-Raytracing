#include "image.h"
#include "log.h"
#include "camera.h"
#include "random.h"
#include "transform.h"
#include "renderer.h"
#include "util/stat.h"
#include "util/resource_finder.h"
#include "util/program_args.h"
#include "geom/bvh.h"
#include "geom/cube.h"
#include "geom/sphere.h"
#include "geom/triangle.h"
#include "geom/static_mesh.h"
#include "loader/obj_loader.h"
#include "loader/image_loader.h"
#include "shading/material.h"
#include "platform/platform.h"

#include <functional>
#include <iostream>
#include <vector>

// oidn is not Windows-only but I'm downloading Windows pre-built binaries.
#if PLATFORM_WINDOWS
	#define INTEL_DENOISER 1
#endif

#if INTEL_DENOISER
#include <oidn.h>
#pragma comment(lib, "OpenImageDenoise.lib")
//#pragma comment(lib, "tbb.lib")
#endif

// -----------------------------------------------------------------------

static ProgramArguments g_programArgs;

// Default rendering configuration
#define CAMERA_APERTURE            0.01f
#define CAMERA_BEGIN_CAPTURE       0.0f
#define CAMERA_END_CAPTURE         5.0f
#define SAMPLES_PER_PIXEL          10
#define MAX_RECURSION              5
#define RAY_T_MIN                  0.0001f

#define DEFAULT_VIEWPORT_WIDTH     1024
#define DEFAULT_VIEWPORT_HEIGHT    512
#define DEFAULT_CAMERA_LOCATION    vec3(0.0f, 0.0f, 0.0f)
#define DEFAULT_CAMERA_LOOKAT      vec3(0.0f, 0.0f, -1.0f)
#define DEFAULT_CAMERA_UP          vec3(0.0f, 1.0f, 0.0f)
#define DEFAULT_FOV_Y              45.0f

vec3 FAKE_SKY_LIGHT(const vec3& dir)
{
	// #todo-lighting: Should be real sky atmosphere
	// that involves interaction with sun light.
	float t = 0.5f * (dir.y + 1.0f);
	return ((1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f));
}
void FAKE_SUN_LIGHT(vec3& outDir, vec3& outIlluminance)
{
	outDir = normalize(vec3(0.0f, -1.0f, -0.5f));
	outIlluminance = vec3(20.0f);
}

// Demo scenes
HitableList* CreateScene_CornellBox();
HitableList* CreateScene_CarShowRoom();
HitableList* CreateScene_BreakfastRoom();
HitableList* CreateScene_DabrovicSponza();
HitableList* CreateScene_FireplaceRoom();
HitableList* CreateScene_LivingRoom();
HitableList* CreateScene_SibenikCathedral();
HitableList* CreateScene_SanMiguel();
HitableList* CreateScene_ObjModel();
HitableList* CreateScene_RandomSpheres();
HitableList* CreateScene_FourSpheres();

struct SceneDesc
{
	std::function<HitableList*(void)> createSceneFn;
	std::string sceneName;
	vec3 cameraLocation = DEFAULT_CAMERA_LOCATION;
	vec3 cameraLookat   = DEFAULT_CAMERA_LOOKAT;
	float fovY          = DEFAULT_FOV_Y;
};

SceneDesc g_sceneDescs[] = {
	{
		CreateScene_CornellBox,
		"CornellBox",
		vec3(0.0f, 1.0f, 4.0f),
		vec3(0.0f, 1.0f, -1.0f),
	},
	{
		CreateScene_BreakfastRoom,
		"BreakfastRoom",
		vec3(0.0f, 1.0f, 5.0f),
		vec3(0.0f, 1.0f, -1.0f),
		60.0f,
	},
	{
		CreateScene_DabrovicSponza,
		"DabrovicSponza",
		vec3(10.0f, 2.0f, 0.0f),
		vec3(0.0f, 3.0f, 0.0f),
		60.0f
	},
	// #todo-obj: Handle all illumination models
	{
		CreateScene_FireplaceRoom,
		"FireplaceRoom",
		vec3(5.0f, 1.0f, -1.5f),
		vec3(0.0f, 1.0f, -1.5f),
		60.0f,
	},
	{
		CreateScene_LivingRoom,
		"LivingRoom",
		vec3(3.0f, 2.0f, 2.0f),
		vec3(0.0f, 1.5f, 2.5f),
		60.0f
	},
	// #todo-obj: Sky light cannnot penetrate opaque windows (need to handle translucency)
	{
		CreateScene_SibenikCathedral,
		"SibenikCathedral",
		vec3(-10.0f, -12.0f, 0.0f),
		vec3(0.0f, -11.5f, 0.0f),
		60.0f
	},
	// Not included in Setup.ps1 as this model is too big.
	{
		CreateScene_SanMiguel,
		"SanMiguel",
		vec3(10.0f, 3.0f, 5.0f),
		vec3(15.0f, 3.0f, 5.0f),
		60.0f
	},
#if 0
	{
		CreateScene_ObjModel,
		"ObjTest",
		vec3(3.0f, 1.0f, 3.0f),
		vec3(0.0f, 1.0f, -1.0f)
	},
	{
		CreateScene_CarShowRoom,
		"CarShowRoom",
		vec3(3.0f, 1.0f, 3.0f),
		vec3(0.0f, 1.0f, -1.0f)
	},
#endif
	{
		CreateScene_FourSpheres,
		"FourSpheres",
		vec3(0.0f, 0.5f, 3.0f),
		vec3(0.0f, 0.5f, 0.0f)
	},
};

// Keep loaded OBJModel instances to avoid reloading them
// when rendering a same scene multiple times.
struct OBJModelContainer
{
	OBJModel* find(const std::string& filename) const
	{
		auto it = modelDB.find(filename);
		if (it == modelDB.end())
		{
			return nullptr;
		}
		return it->second;
	}
	void insert(const std::string& filename, OBJModel* model)
	{
		modelDB.insert(std::make_pair(filename, model));
	}
	void clear()
	{
		for (auto& it : modelDB)
		{
			OBJModel* model = it.second;
			delete model;
		}
		modelDB.clear();
	}

	std::map<std::string, OBJModel*> modelDB;
};

OBJModelContainer g_objContainer;

void InitializeSubsystems() {
	SCOPED_CPU_COUNTER(InitializeSubsystems);

	StartLogThread();
	ResourceFinder::Get().AddDirectory("./content/");
	ImageLoader::Initialize();
	OBJLoader::Initialize();
}
void DestroySubsystems() {
	OBJLoader::Destroy();
	ImageLoader::Destroy();
	WaitForLogThread();
}

void ExecuteRenderer(uint32 sceneID, const RendererSettings& settings);

int main(int argc, char** argv) {
	LOG("=== Software Raytracer ===");

	//
	// Initialize
	//
	g_programArgs.init(argc, argv);
	InitializeSubsystems();

	uint32 currentSceneID = 0;

	RendererSettings rendererSettings;
	rendererSettings.viewportWidth   = DEFAULT_VIEWPORT_WIDTH;
	rendererSettings.viewportHeight  = DEFAULT_VIEWPORT_HEIGHT;
	rendererSettings.cameraLocation  = g_sceneDescs[currentSceneID].cameraLocation;
	rendererSettings.cameraLookat    = g_sceneDescs[currentSceneID].cameraLookat;
	rendererSettings.samplesPerPixel = SAMPLES_PER_PIXEL;
	rendererSettings.maxPathLength   = MAX_RECURSION;
	rendererSettings.rayTMin         = RAY_T_MIN;
	rendererSettings.skyLightFn      = FAKE_SKY_LIGHT;
	rendererSettings.sunLightFn      = FAKE_SUN_LIGHT;
	rendererSettings.debugMode       = EDebugMode::None;
	rendererSettings.bRunDenoiser    = INTEL_DENOISER;

	FlushLogThread();
	std::cout << "Type 'help' to see help message" << std::endl;

	while (true)
	{
		std::cin.clear();
		std::cout << "> ";

		std::string command;
		std::cin >> command;
		if (command == "help")
		{
			std::cout << "list         : print all default scene descs" << std::endl;
			std::cout << "select n     : select a scene to render (also reset the camera)" << std::endl;
			std::cout << "run          : render the scene currently selected" << std::endl;
			std::cout << "denoiser n   : toggle denoiser (0/1)" << std::endl;
			std::cout << "spp n        : set samplers per pixel" << std::endl;
			std::cout << "viewport w h : set viewport size" << std::endl;
			std::cout << "moveto x y z : change camera location" << std::endl;
			std::cout << "lookat x y z : change camera lookat" << std::endl;
			std::cout << "viewmode n   : change viewmode (enter -1 to see help)" << std::endl;
			std::cout << "exit         : exit the program" << std::endl;
		}
		else if (command == "list")
		{
			for (size_t i = 0; i < _countof(g_sceneDescs); ++i)
			{
				std::cout << i << " - " << g_sceneDescs[i].sceneName << std::endl;
			}
		}
		else if (command == "select")
		{
			uint32 prevSceneID = currentSceneID;

			std::cin >> currentSceneID;
			if (std::cin.good())
			{
				if (currentSceneID >= _countof(g_sceneDescs)) currentSceneID = 0;
				std::cout << "select: " << g_sceneDescs[currentSceneID].sceneName << std::endl;
			}
			else
			{
				std::cout << "Invalid scene number" << std::endl;
			}

			rendererSettings.cameraLocation = g_sceneDescs[currentSceneID].cameraLocation;
			rendererSettings.cameraLookat = g_sceneDescs[currentSceneID].cameraLookat;
			if (prevSceneID != currentSceneID)
			{
				g_objContainer.clear();
			}
		}
		else if (command == "run")
		{
			ExecuteRenderer(currentSceneID, rendererSettings);
		}
		else if (command == "denoiser")
		{
			uint32 dmode;
			std::cin >> dmode;
#if INTEL_DENOISER
			rendererSettings.bRunDenoiser = (dmode != 0);
			std::cout << "Denoiser " << (rendererSettings.bRunDenoiser ? "on" : "off") << std::endl;
#else
			std::cout << "Denoiser was not integrated" << std::endl;
#endif
		}
		else if (command == "spp")
		{
			uint32 spp;
			std::cin >> spp;
			if (std::cin.good())
			{
				rendererSettings.samplesPerPixel = std::max(1u, spp);
			}
			else
			{
				std::cout << "Invalid SPP" << std::endl;
			}
		}
		else if (command == "viewport")
		{
			uint32 w, h;
			std::cin >> w >> h;
			if (std::cin.good())
			{
				rendererSettings.viewportWidth = w;
				rendererSettings.viewportHeight = h;
			}
			else
			{
				std::cout << "Invalid viewport size" << std::endl;
			}
		}
		else if (command == "moveto")
		{
			float x, y, z;
			std::cin >> x >> y >> z;
			if (std::cin.good())
			{
				rendererSettings.cameraLocation = vec3(x, y, z);
			}
			else
			{
				std::cout << "Invalid camera location" << std::endl;
			}
		}
		else if (command == "lookat")
		{
			float x, y, z;
			std::cin >> x >> y >> z;
			if (std::cin.good())
			{
				rendererSettings.cameraLookat = vec3(x, y, z);
			}
			else
			{
				std::cout << "Invalid camera lookat" << std::endl;
			}
		}
		else if (command == "viewmode")
		{
			int32 vmode;
			std::cin >> vmode;
			if (std::cin.good())
			{
				const char* vmodeStr = GetRendererDebugModeString(vmode);
				if (0 <= vmode && vmode < (int32)EDebugMode::MAX)
				{
					rendererSettings.debugMode = (EDebugMode)vmode;
					std::cout << "Set viewmode = " << vmodeStr << std::endl;
				}
				else
				{
					for (int32 i = 0; i < (int32)EDebugMode::MAX; ++i)
					{
						std::cout << i << " - " << GetRendererDebugModeString(i) << std::endl;
					}
				}
			}
			else
			{
				std::cout << "Invalid viewmode; please enter a number" << std::endl;
			}
		}
		else if (command == "exit")
		{
			break;
		}
		else
		{
			std::cout << "Unknown command" << std::endl;
			std::cout << "Type 'help' to see help message" << std::endl;
		}
	}

	//
	// Cleanup
	//
	DestroySubsystems();

	return 0;
}

void ExecuteRenderer(uint32 sceneID, const RendererSettings& settings)
{
	const SceneDesc& sceneDesc = g_sceneDescs[sceneID];
	
	auto makeFilename = [&sceneDesc](const char* prefix) {
		std::string name = SOLUTION_DIR "test_";
		name += sceneDesc.sceneName;
		name += prefix;
		return name;
	};

	LOG("Execute renderer for: %s", sceneDesc.sceneName.c_str());

	BVHNode* worldBVH = new BVHNode(
		sceneDesc.createSceneFn(), CAMERA_BEGIN_CAPTURE, CAMERA_END_CAPTURE);

	Camera camera(
		settings.cameraLocation,
		settings.cameraLookat,
		DEFAULT_CAMERA_UP,
		sceneDesc.fovY,
		settings.getViewportAspectWH(),
		CAMERA_APERTURE,
		(settings.cameraLocation - settings.cameraLookat).Length(),
		CAMERA_BEGIN_CAPTURE,
		CAMERA_END_CAPTURE);

	// Render default image
	const uint32 viewportWidth = settings.viewportWidth;
	const uint32 viewportHeight = settings.viewportHeight;
	Image2D image(viewportWidth, viewportHeight);
	Renderer renderer;
	renderer.RenderScene(settings, worldBVH, &camera, &image);
	// NOTE: I'll input HDR image to denoiser

	if (settings.bRunDenoiser && settings.debugMode == EDebugMode::None)
	{
#if INTEL_DENOISER
		// Render aux images
		Image2D imageWorldNormal(viewportWidth, viewportHeight);
		Image2D imageAlbedo(viewportWidth, viewportHeight);

		Camera debugCamera = camera;
		debugCamera.lens_radius = 0.0f;

		RendererSettings debugSettings = settings;
		debugSettings.debugMode = EDebugMode::Reflectance;
		renderer.RenderScene(debugSettings, worldBVH, &debugCamera, &imageAlbedo);
		debugSettings.debugMode = EDebugMode::VertexNormal;
		renderer.RenderScene(debugSettings, worldBVH, &debugCamera, &imageWorldNormal);

		std::string albedoFilenameJPG = makeFilename("_0.jpg");
		std::string normalFilenameJPG = makeFilename("_1.jpg");
		WriteImageToDisk(imageAlbedo, albedoFilenameJPG.c_str(), EImageFileType::Jpg);
		WriteImageToDisk(imageWorldNormal, normalFilenameJPG.c_str(), EImageFileType::Jpg);

		// Dump as float array to pass to oidn
		std::vector<float> noisyInputBlob, albedoBlob, worldNormalBlob;
		image.DumpFloatRGBs(noisyInputBlob);
		imageAlbedo.DumpFloatRGBs(albedoBlob);
		imageWorldNormal.DumpFloatRGBs(worldNormalBlob);

		LOG("Denoise the result using Intel OpenImageDenoise");

		OIDNDevice device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);
		oidnCommitDevice(device);

		std::vector<float> denoisedOutputBlob(noisyInputBlob.size(), 0.0f);

		OIDNFilter filter = oidnNewFilter(device, "RT");
		oidnSetSharedFilterImage(filter, "color", noisyInputBlob.data(),
			OIDN_FORMAT_FLOAT3, viewportWidth, viewportHeight, 0, 0, 0); // noisy input
		oidnSetSharedFilterImage(filter, "albedo", albedoBlob.data(),
			OIDN_FORMAT_FLOAT3, viewportWidth, viewportHeight, 0, 0, 0); // noisy input
		oidnSetSharedFilterImage(filter, "normal", worldNormalBlob.data(),
			OIDN_FORMAT_FLOAT3, viewportWidth, viewportHeight, 0, 0, 0); // noisy input
		oidnSetSharedFilterImage(filter, "output", denoisedOutputBlob.data(),
			OIDN_FORMAT_FLOAT3, viewportWidth, viewportHeight, 0, 0, 0); // noisy input
		oidnSetFilter1b(filter, "hdr", true);
		oidnCommitFilter(filter);

		oidnExecuteFilter(filter);

		const char* oidnErr;
		if (oidnGetDeviceError(device, &oidnErr) != OIDN_ERROR_NONE)
		{
			LOG("oidn error: %s", oidnErr);
		}

		oidnReleaseFilter(filter);
		oidnReleaseDevice(device);

		Image2D denoisedOutput(viewportWidth, viewportHeight);
		size_t k = 0;
		for (uint32_t y = 0; y < viewportHeight; ++y)
		{
			for (uint32_t x = 0; x < viewportWidth; ++x)
			{
				Pixel px(denoisedOutputBlob[k], denoisedOutputBlob[k + 1], denoisedOutputBlob[k + 2]);
				denoisedOutput.SetPixel(x, y, px);
				k += 3;
			}
		}
		denoisedOutput.PostProcess();

		std::string denoiseFilenameJPG = makeFilename("_2.jpg");
		WriteImageToDisk(denoisedOutput, denoiseFilenameJPG.c_str(), EImageFileType::Jpg);
	}
#endif // INTEL_DENOISER

	// Apply postprocessing after denoising
	image.PostProcess();
	std::string resultFilenameBMP = makeFilename(".bmp");
	std::string resultFilenameJPG = makeFilename(".jpg");
	WriteImageToDisk(image, resultFilenameBMP.c_str(), EImageFileType::Bitmap);
	WriteImageToDisk(image, resultFilenameJPG.c_str(), EImageFileType::Jpg);
	LOG("image has been written to: %s", resultFilenameBMP.c_str());
	LOG("image has been written to: %s", resultFilenameJPG.c_str());

	delete worldBVH;

	LOG("=== Rendering has completed ===");
	FlushLogThread();
}

//////////////////////////////////////////////////////////////////////////
// Demo scene generator functions

using OBJTransformer = std::function<void(OBJModel* inModel)>;
bool GetOrCreateOBJ(const char* filename, OBJModel*& outModel, OBJTransformer transformer = nullptr)
{
	OBJModel* model = g_objContainer.find(filename);
	if (model != nullptr)
	{
		outModel = model;
		return true;
	}
	model = new OBJModel;
	if (OBJLoader::SyncLoad(filename, *model))
	{
		if (transformer)
		{
			transformer(model);
		}
		model->FinalizeAllMeshes();
		g_objContainer.insert(filename, model);
		outModel = model;
		return true;
	}
	delete model;
	model = outModel = nullptr;
	return false;
}

HitableList* CreateScene_CornellBox() {
	SCOPED_CPU_COUNTER(CreateScene_CornellBox);

	std::vector<Hitable*> list;

	OBJModel* objModel;
	if (GetOrCreateOBJ("content/cornell_box/CornellBox-Mirror.obj", objModel))
	{
		list.push_back(objModel->rootObject);
	}

	return new HitableList(list);
}

HitableList* CreateScene_CarShowRoom()
{
	SCOPED_CPU_COUNTER(CreateRandomScene);

	std::vector<Hitable*> list;

	// #todo-showroom: Include bmw-m6 model from https://www.pbrt.org/scenes-v3
	// to make this scene a real show room.

	const int32 numCols = 16;
	const int32 numRows = 8;
	const float minRadius = 1.0f;
	const float maxRadius = 1.8f;
	const float height = 2.0f;

	const float deltaPhi = (2.0f * (float)M_PI) / (float)numCols;
	const float deltaHeight = height / (float)numRows;

	auto RadiiFn = [](float r0, float r1, float t) -> float {
		t = 2.0f * t - 1.0f;
		t = 1.0f - sqrtf(1.0f - t * t);
		return r0 + t * (r1 - r0);
	};

#define USE_STATIC_MESH_PILLAR 1

#if USE_STATIC_MESH_PILLAR
	StaticMesh* pillar = new StaticMesh;
	list.push_back(pillar);
#endif

	for (int32 row = 0; row < numRows; ++row) {
		float z0 = height * (float)row / numRows;
		float z1 = z0 + 0.95f * deltaHeight;
		float radius0 = RadiiFn(minRadius, maxRadius, (float)row / numRows);
		float radius1 = RadiiFn(minRadius, maxRadius, (float)(row + 1) / numRows);
		for (int32 col = 0; col < numCols; ++col) {
			float phi = (float)col * deltaPhi;
			float phiNext = phi + 0.95f * deltaPhi;
			float x0 = cosf(phi);
			float y0 = sinf(phi);
			float x1 = cosf(phiNext);
			float y1 = sinf(phiNext);

			vec3 v0 = vec3(radius0, 1.0f, radius0) * vec3(x0, z0, y0);
			vec3 v1 = vec3(radius0, 1.0f, radius0) * vec3(x1, z0, y1);
			vec3 v2 = vec3(radius1, 1.0f, radius1) * vec3(x0, z1, y0);
			vec3 v3 = vec3(radius1, 1.0f, radius1) * vec3(x1, z1, y1);

			vec3 p0 = (v0 + v1) / 2.0f;
			vec3 p1 = (v1 + v3) / 2.0f;
			vec3 p2 = (v2 + v0) / 2.0f;
			vec3 p3 = (v3 + v2) / 2.0f;
			v0 = p0; v1 = p1; v2 = p2; v3 = p3;

			vec3 n0 = normalize(cross(v1 - v0, v2 - v0));
			vec3 n1 = normalize(cross(v3 - v1, v0 - v1));
			vec3 n2 = normalize(cross(v0 - v2, v3 - v2));
			vec3 n3 = normalize(cross(v2 - v3, v1 - v3));

			//Material* mat = new Lambertian(0.2f + 0.75f * abs(RandomInUnitSphere()));
			MicrofacetMaterial* mat = new MicrofacetMaterial;
			mat->SetAlbedoFallback(0.2f + 0.75f * abs(RandomInUnitSphere()));
			mat->SetRoughnessFallback(0.1f);
			//mat->SetMetallicFallback(1.0f);

#if USE_STATIC_MESH_PILLAR
			pillar->AddTriangle(Triangle(v0, v1, v2, n0, n1, n2, mat));
			pillar->AddTriangle(Triangle(v1, v3, v2, n1, n3, n2, mat));
#else
			Triangle* tri0 = new Triangle(v0, v1, v2, n0, n1, n2, mat);
			Triangle* tri1 = new Triangle(v1, v3, v2, n1, n3, n2, mat);
			list.push_back(tri0);
			list.push_back(tri1);
#endif
		}
	}
	for (int32 row = 0; row < numRows; ++row) {
		float row2 = (float)row + 0.5f;
		float z0 = height * ((float)row + 0.5f) / numRows;
		float z1 = z0 + 0.95f * deltaHeight;
		float radius0 = RadiiFn(minRadius, maxRadius, row2 / numRows);
		float radius1 = RadiiFn(minRadius, maxRadius, (row2 + 1.0f) / numRows);
		for (int32 col = 0; col < numCols; ++col) {
			float col2 = (float)col + 0.5f;
			float phi = col2 * deltaPhi;
			float phiNext = phi + 0.95f * deltaPhi;
			float x0 = cosf(phi);
			float y0 = sinf(phi);
			float x1 = cosf(phiNext);
			float y1 = sinf(phiNext);

			vec3 v0 = vec3(radius0, 1.0f, radius0) * vec3(x0, z0, y0);
			vec3 v1 = vec3(radius0, 1.0f, radius0) * vec3(x1, z0, y1);
			vec3 v2 = vec3(radius1, 1.0f, radius1) * vec3(x0, z1, y0);
			vec3 v3 = vec3(radius1, 1.0f, radius1) * vec3(x1, z1, y1);

			vec3 p0 = (v0 + v1) / 2.0f;
			vec3 p1 = (v1 + v3) / 2.0f;
			vec3 p2 = (v2 + v0) / 2.0f;
			vec3 p3 = (v3 + v2) / 2.0f;
			v0 = p0; v1 = p1; v2 = p2; v3 = p3;

			vec3 n0 = normalize(cross(v1 - v0, v2 - v0));
			vec3 n1 = normalize(cross(v3 - v1, v0 - v1));
			vec3 n2 = normalize(cross(v0 - v2, v3 - v2));
			vec3 n3 = normalize(cross(v2 - v3, v1 - v3));

			//Material* mat = new Lambertian(0.2f + 0.75f * abs(RandomInUnitSphere()));
			MicrofacetMaterial* mat = new MicrofacetMaterial;
			//mat->SetAlbedoFallback(0.2f + 0.75f * abs(RandomInUnitSphere()));
			mat->SetAlbedoFallback(vec3(0.9f));
			mat->SetRoughnessFallback(0.1f);
			mat->SetMetallicFallback(1.0f);

#if USE_STATIC_MESH_PILLAR
			pillar->AddTriangle(Triangle(v0, v1, v2, n0, n1, n2, mat));
			pillar->AddTriangle(Triangle(v1, v3, v2, n1, n3, n2, mat));
#else
			Triangle* tri0 = new Triangle(v0, v1, v2, n0, n1, n2, mat);
			Triangle* tri1 = new Triangle(v1, v3, v2, n1, n3, n2, mat);
			list.push_back(tri0);
			list.push_back(tri1);
#endif
		}
	}

#if USE_STATIC_MESH_PILLAR
	pillar->Finalize();
#endif

	list.push_back(new sphere(vec3(3.0f, 1.0f, 0.0f), 1.0f, new Lambertian(vec3(0.9f, 0.2f, 0.2f))));
	list.push_back(new sphere(vec3(-3.0f, 1.0f, 0.0f), 1.0f, new Lambertian(vec3(0.2f, 0.9f, 0.2f))));
	list.push_back(new sphere(vec3(0.0f, 1.0f, 3.0f), 1.0f, new Lambertian(vec3(0.2f, 0.2f, 0.9f))));

	// Ground
	sphere* ground = new sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new Lambertian(vec3(0.5f, 0.5f, 0.5f)));
	list.push_back(ground);

	return new HitableList(list);
}

HitableList* CreateScene_BreakfastRoom()
{
	SCOPED_CPU_COUNTER(CreateScene_BreakfastRoom);

	std::vector<Hitable*> list;

	OBJModel* objModel;
	if (GetOrCreateOBJ("content/breakfast_room/breakfast_room.obj", objModel))
	{
		list.push_back(objModel->rootObject);
	}

	return new HitableList(list);
}

HitableList* CreateScene_DabrovicSponza()
{
	SCOPED_CPU_COUNTER(CreateScene_DabrovicSponza);

	std::vector<Hitable*> list;

	OBJModel* objModel;
	if (GetOrCreateOBJ("content/dabrovic_sponza/sponza.obj", objModel))
	{
		list.push_back(objModel->rootObject);
	}

	return new HitableList(list);
}

HitableList* CreateScene_FireplaceRoom()
{
	SCOPED_CPU_COUNTER(CreateScene_FireplaceRoom);

	std::vector<Hitable*> list;

	OBJModel* objModel;
	if (GetOrCreateOBJ("content/fireplace_room/fireplace_room.obj", objModel))
	{
		list.push_back(objModel->rootObject);
	}

	return new HitableList(list);
}

HitableList* CreateScene_LivingRoom()
{
	SCOPED_CPU_COUNTER(CreateScene_LivingRoom);

	std::vector<Hitable*> list;

	OBJModel* objModel;
	if (GetOrCreateOBJ("content/living_room/living_room.obj", objModel))
	{
		list.push_back(objModel->rootObject);
	}

	return new HitableList(list);
}

HitableList* CreateScene_SibenikCathedral()
{
	SCOPED_CPU_COUNTER(CreateScene_SibenikCathedral);

	std::vector<Hitable*> list;

	OBJModel* objModel;
	if (GetOrCreateOBJ("content/sibenik/sibenik.obj", objModel))
	{
		list.push_back(objModel->rootObject);
	}

	return new HitableList(list);
}

HitableList* CreateScene_SanMiguel()
{
	SCOPED_CPU_COUNTER(CreateScene_SanMiguel);

	std::vector<Hitable*> list;

	OBJModel* objModel;
	if (GetOrCreateOBJ("content/San_Miguel/san-miguel-low-poly.obj", objModel))
	{
		list.push_back(objModel->rootObject);
	}

	return new HitableList(list);
}

HitableList* CreateScene_ObjModel()
{
	SCOPED_CPU_COUNTER(CreateRandomScene);

// #todo-wip: Magic values only for CreateScene_ObjModel()
#define OBJTEST_LOCAL_LIGHTS            1
#define OBJTEST_INCLUDE_TOADTTE         1
#define OBJTEST_INCLUDE_CUBE            1

	std::vector<Hitable*> list;

	// Light source
#if OBJTEST_LOCAL_LIGHTS
	Material* pointLight0 = new DiffuseLight(vec3(5.0f, 0.0f, 0.0f));
	Material* pointLight1 = new DiffuseLight(vec3(0.0f, 4.0f, 5.0f));
	list.push_back(new sphere(vec3(2.0f, 2.0f, 0.0f), 0.5f, pointLight0));
	list.push_back(new sphere(vec3(-1.0f, 2.0f, 1.0f), 0.3f, pointLight1));
#endif

#if OBJTEST_INCLUDE_TOADTTE
	OBJModel* model;
	auto transformer = [](OBJModel* inModel) {
		Transform transform;
		transform.Init(
			vec3(0.0f, 0.0f, 0.0f),
			Rotator(-10.0f, 0.0f, 0.0f),
			vec3(0.07f, 0.07f, 0.07f));
		std::for_each(
			inModel->staticMeshes.begin(),
			inModel->staticMeshes.end(),
			[&transform](StaticMesh* mesh) { mesh->ApplyTransform(transform); }
		);
	};
	if (GetOrCreateOBJ("content/Toadette/Toadette.obj", model))
	{
		list.push_back(model->rootObject);
	}
#endif

#if OBJTEST_INCLUDE_CUBE
	Material* cube_mat = new Lambertian(vec3(0.9f, 0.1f, 0.1f));
	Material* cube_mat2 = new Lambertian(vec3(0.1f, 0.1f, 0.9f));
	list.push_back(new Cube(vec3(-4.0f, 0.3f, 0.0f), vec3(-3.0f, 0.5f, 1.0f), CAMERA_BEGIN_CAPTURE, vec3(0.0f, 0.05f, 0.0f), cube_mat));
	list.push_back(new Cube(vec3(-5.5f, 0.0f, 0.0f), vec3(-4.5f, 2.0f, 2.0f), CAMERA_BEGIN_CAPTURE, vec3(0.0f, 0.05f, 0.0f), cube_mat2));
#endif

	std::shared_ptr<Image2D> img;
	if (ImageLoader::SyncLoad("content/Toadette/Toadette_body.png", img))
	{
		MicrofacetMaterial* pbr_mat = new MicrofacetMaterial;
		pbr_mat->SetAlbedoTexture(img);

		const vec3 origin(1.0f, 0.0f, 0.0f);
		const vec3 n(0.0f, 0.0f, 1.0f);
 		{
			Triangle* T = new Triangle(
				origin + vec3(0.0f, 0.0f, 0.0f), origin + vec3(1.0f, 0.0f, 0.0f), origin + vec3(1.0f, 1.0f, 0.0f),
				n, n, n, pbr_mat);
 			T->SetParameterization(0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
			list.push_back(T);
 		}
 		{
 			Triangle* T = new Triangle(
				origin + vec3(0.0f, 0.0f, 0.0f), origin + vec3(1.0f, 1.0f, 0.0f), origin + vec3(0.0f, 1.0f, 0.0f),
				n, n, n, pbr_mat);
 			T->SetParameterization(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
			list.push_back(T);
 		}
	}

	const int32 numFans = 8;
	const float fanAngle = 1.0f / (float)(numFans + 1);
	for (int32 i = 0; i <= numFans; ++i)
	{
		float fanBegin = pi<float>* fanAngle* i;
		float fanEnd = pi<float> * fanAngle * (i + 1);
		float fanRadius = 3.0f + 1.0f * (float)i / numFans;
		float z = -2.0f;
		vec3 v0(fanRadius * std::cos(fanBegin), fanRadius * std::sin(fanBegin), z);
		vec3 v1(0.0f, 0.0f, z);
		vec3 v2(fanRadius * std::cos(fanEnd), fanRadius * std::sin(fanEnd), z);
		vec3 n(0.0f, 0.0f, 1.0f);

		vec3 color = vec3(0.3f, 0.3f, 0.3f) + 0.6f * abs(RandomInUnitSphere());
#if 0
		Material* mat = new Lambertian(color);
#else
		MicrofacetMaterial* mat = new MicrofacetMaterial;
		mat->SetAlbedoFallback(color);
		//mat->SetAlbedoFallback(vec3(0.9f));
		mat->SetRoughnessFallback(0.05f);
		//mat->SetMetallicFallback(1.0f);
#endif

		list.push_back(new Triangle(v0, v1, v2, n, n, n, mat));
	}

	// Ground
	list.push_back(new sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new Lambertian(vec3(0.5f, 0.5f, 0.5f))));

	return new HitableList(list);
}

HitableList* CreateScene_RandomSpheres()
{
	std::vector<Hitable*> list;

	list.push_back(new sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new Lambertian(vec3(0.5f, 0.5f, 0.5f))));

	for(int32 a = -6; a < 6; ++a)
	{
		for(int32 b = -6; b < 6; ++b)
		{
			float choose_material = Random();
			vec3 center(a + 0.9f * Random(), 0.2f, b + 0.9f * Random());
			if((center - vec3(4.0f, 0.2f, 0.0f)).Length() > 2.0f)
			{
				if(choose_material < 0.8f)
				{
					list.push_back(new sphere(center, 0.2f,
						new Lambertian(vec3(Random()*Random(), Random()*Random(), Random()*Random()))));
				}
				else if(choose_material < 0.95f)
				{
					list.push_back(new sphere(center, 0.2f,
						new Metal(vec3(0.5f * (1.0f + Random()), 0.5f * (1.0f + Random()), 0.5f * (1.0f + Random())), 0.5f * Random())));
				}
				else
				{
					list.push_back(new sphere(center, 0.2f, new Dielectric(1.5f)));
				}
			}
		}
	}
	list.push_back(new sphere(vec3(0.0f, 1.0f, 0.0f), 1.0f, new Dielectric(1.5f)));
	list.push_back(new sphere(vec3(-2.0f, 1.0f, 0.0f), 1.0f, new Lambertian(vec3(0.4f, 0.2f, 0.1f))));
	list.push_back(new sphere(vec3(2.0f, 1.0f, 0.0f), 1.0f, new Metal(vec3(0.7f, 0.6f, 0.5f), 0.0f)));

	return new HitableList(list);
}

HitableList* CreateScene_FourSpheres()
{
	//Material* M_ground = new Lambertian(vec3(0.8f, 0.8f, 0.0f));
	float groundRoughness = 1.0f; // #todo-wip: Debug microfacet BRDF with roughness = 0
	Material* M_ground = MicrofacetMaterial::FromConstants(
		vec3(1.0f), groundRoughness, 0.0f, vec3(0.0f));

	Material* M_left = new Dielectric(1.0f, vec3(1.0f, 0.5f, 0.5f));
	Material* M_center = new Lambertian(vec3(0.8f, 0.3f, 0.3f));
	Material* M_right = new Metal(vec3(0.8f, 0.6f, 0.2f), 0.0f);

	std::vector<Hitable*> list;
	list.push_back(new sphere(vec3(0.0f, -100.5f, -1.0f), 100.0f, M_ground));
	list.push_back(new sphere(vec3(-1.0f, 0.0f, -1.0f), 0.5f, M_left));
	list.push_back(new sphere(vec3(0.0f, 0.0f, -1.0f), 0.5f, M_center));
	list.push_back(new sphere(vec3(1.0f, 0.0f, -1.0f), 0.5f, M_right));
	HitableList* world = new HitableList(list);
	return world;
}
