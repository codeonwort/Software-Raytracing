#include "program_args.h"
#include "resource_finder.h"

// #todo-raylib: All belongs to raylib
#include "core/random.h"
#include "render/material.h"
#include "geom/cube.h"
#include "geom/sphere.h"
#include "geom/triangle.h"
#include "geom/static_mesh.h"

#include "raylib/raylib.h"

#include <functional>
#include <iostream>
#include <vector>
#include <map>

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
#define DEFAULT_FOV_Y              45.0f

ImageHandle skyPanorama = NULL;

// Demo scenes
SceneHandle CreateScene_CornellBox();
SceneHandle CreateScene_CarShowRoom();
SceneHandle CreateScene_BreakfastRoom();
SceneHandle CreateScene_DabrovicSponza();
SceneHandle CreateScene_FireplaceRoom();
SceneHandle CreateScene_LivingRoom();
SceneHandle CreateScene_SibenikCathedral();
SceneHandle CreateScene_SanMiguel();
SceneHandle CreateScene_ObjModel();
SceneHandle CreateScene_RandomSpheres();
SceneHandle CreateScene_FourSpheres();

struct SceneDesc
{
	std::function<SceneHandle(void)> createSceneFn;
	std::string sceneName;
	vec3 cameraLocation             = DEFAULT_CAMERA_LOCATION;
	vec3 cameraLookat               = DEFAULT_CAMERA_LOOKAT;
	float fovY                      = DEFAULT_FOV_Y;
	bool bUseSkyImage               = false;
	vec3 sunIlluminance             = vec3(20.0f);
	vec3 sunDirection               = normalize(vec3(0.0f, -1.0f, -0.5f));
};

SceneDesc g_sceneDescs[] = {
	{
		CreateScene_CornellBox,
		"CornellBox",
		vec3(0.0f, 1.0f, 4.0f),
		vec3(0.0f, 1.0f, -1.0f),
		DEFAULT_FOV_Y,
		false,
		vec3(0.0f) // Sun
	},
	{
		CreateScene_BreakfastRoom,
		"BreakfastRoom",
		vec3(0.0f, 1.0f, 5.0f),
		vec3(0.0f, 1.0f, -1.0f),
		60.0f,
		true,
		vec3(20.0f), vec3(-1.0f, -1.0f, 0.0f) // Sun
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
	{
		CreateScene_SanMiguel,
		"SanMiguel",
		vec3(10.0f, 3.0f, 5.0f),
		vec3(15.0f, 3.0f, 5.0f),
		60.0f
	},
	{
		CreateScene_FourSpheres,
		"FourSpheres",
		vec3(0.0f, 0.5f, 3.0f),
		vec3(0.0f, 0.5f, 0.0f),
		DEFAULT_FOV_Y,
		true,
		vec3(0.0f)
	},
	{
		CreateScene_RandomSpheres,
		"RandomSpheres",
		vec3(0.0f, 1.5f, 5.0f),
		vec3(0.0f, 0.5f, 0.0f),
		60.0f,
		true,
		vec3(0.0f)
	},
#if 0
	{
		CreateScene_ObjModel,
		"ObjTest",
		vec3(3.0f, 1.0f, 3.0f),
		vec3(0.0f, 1.0f, -1.0f),
		DEFAULT_FOV_Y,
		nullptr, // no sky light
		nullptr  // no sun light
	},
	{
		CreateScene_CarShowRoom,
		"CarShowRoom",
		vec3(3.0f, 1.0f, 3.0f),
		vec3(0.0f, 1.0f, -1.0f)
	},
#endif
};

// Keep loaded OBJModel instances to avoid reloading them
// when rendering a same scene multiple times.
struct OBJModelContainer
{
	OBJModelHandle find(const std::string& filename) const
	{
		auto it = modelDB.find(filename);
		if (it == modelDB.end())
		{
			return NULL;
		}
		return it->second;
	}
	void insert(const std::string& filename, OBJModelHandle model)
	{
		modelDB.insert(std::make_pair(filename, model));
	}
	void clear()
	{
		for (auto& it : modelDB)
		{
			OBJModelHandle model = it.second;
			Raylib_UnloadOBJModel(model);
		}
		modelDB.clear();
	}

	std::map<std::string, OBJModelHandle> modelDB;
};

OBJModelContainer g_objContainer;

void ExecuteRenderer(
	uint32 sceneID,
	bool bRunDenoiser,
	const vec3& cameraLocation,
	const vec3& cameraLookAt,
	const RendererSettings& settings);

int main(int argc, char** argv) {
	LOG("=== Software Raytracer ===");

	//
	// Initialize
	//
	g_programArgs.init(argc, argv);
	if (Raylib_Initialize() == false)
	{
		std::cerr << "Failed to initialize raylib" << std::endl;
		return -1;
	}
	ResourceFinder::Get().AddDirectory("./content/");

	uint32 currentSceneID = 0;
	bool bRunDenoiser = true;

	vec3 cameraLocation = g_sceneDescs[currentSceneID].cameraLocation;
	vec3 cameraLookAt = g_sceneDescs[currentSceneID].cameraLookat;

	RendererSettings rendererSettings;
	rendererSettings.viewportWidth   = DEFAULT_VIEWPORT_WIDTH;
	rendererSettings.viewportHeight  = DEFAULT_VIEWPORT_HEIGHT;
	rendererSettings.samplesPerPixel = SAMPLES_PER_PIXEL;
	rendererSettings.maxPathLength   = MAX_RECURSION;
	rendererSettings.rayTMin         = RAY_T_MIN;
	rendererSettings.renderMode      = ERenderMode::RAYLIB_RENDERMODE_Default;

	Raylib_FlushLogThread();
	std::cout << "Type 'help' to see help message" << std::endl;

	//
	// Read-Eval-Print loop
	//
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

			cameraLocation = g_sceneDescs[currentSceneID].cameraLocation;
			cameraLookAt = g_sceneDescs[currentSceneID].cameraLookat;
			if (prevSceneID != currentSceneID)
			{
				g_objContainer.clear();
			}
		}
		else if (command == "run")
		{
			ExecuteRenderer(
				currentSceneID,
				bRunDenoiser,
				cameraLocation,
				cameraLookAt,
				rendererSettings);
		}
		else if (command == "denoiser")
		{
			uint32 dmode;
			std::cin >> dmode;
			if (std::cin.good())
			{
				if (Raylib_IsDenoiserSupported())
				{
					bRunDenoiser = (dmode != 0);
					std::cout << "Denoiser " << (bRunDenoiser ? "on" : "off") << std::endl;
				}
				else
				{
					std::cout << "Denoiser was not integrated" << std::endl;
				}
			}
			else
			{
				std::cout << "Invalid denoiser option, current=" << (int32)bRunDenoiser << std::endl;
			}
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
				cameraLocation = vec3(x, y, z);
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
				cameraLookAt = vec3(x, y, z);
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
				const char* vmodeStr = Raylib_GetRenderModeString(vmode);
				if (0 <= vmode && vmode < (int32)ERenderMode::RAYLIB_RENDERMODE_MAX)
				{
					rendererSettings.renderMode = (ERenderMode)vmode;
					std::cout << "Set viewmode = " << vmodeStr << std::endl;
				}
				else
				{
					std::cout << "Invalid viewmode. Current: " << rendererSettings.renderMode << std::endl;
					for (int32 i = 0; i < (int32)ERenderMode::RAYLIB_RENDERMODE_MAX; ++i)
					{
						std::cout << i << " - " << Raylib_GetRenderModeString(i) << std::endl;
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
	if (skyPanorama != NULL)
	{
		Raylib_DestroyImage(skyPanorama);
		skyPanorama = NULL;
	}
	Raylib_Terminate();

	return 0;
}

void ExecuteRenderer(
	uint32 sceneID,
	bool bRunDenoiser,
	const vec3& cameraLocation,
	const vec3& cameraLookAt,
	const RendererSettings& settings)
{
	const SceneDesc& sceneDesc = g_sceneDescs[sceneID];

	if (skyPanorama == NULL)
	{
		std::string filepath = ResourceFinder::Get().Find("content/Ridgecrest_Road_Ref.hdr");
		skyPanorama = Raylib_LoadImage(filepath.c_str());
	}
	
	auto makeFilename = [&sceneDesc](const char* prefix) {
		std::string name = SOLUTION_DIR "test_";
		name += sceneDesc.sceneName;
		name += prefix;
		return name;
	};

	LOG("Execute renderer for: %s", sceneDesc.sceneName.c_str());

	const uint32 viewportWidth = settings.viewportWidth;
	const uint32 viewportHeight = settings.viewportHeight;
	const float focalDistance = (cameraLocation - cameraLookAt).Length();

	SceneHandle scene = sceneDesc.createSceneFn();
	Raylib_SetSkyPanorama(scene, sceneDesc.bUseSkyImage ? skyPanorama : NULL);
	Raylib_SetSunIlluminance(scene, sceneDesc.sunIlluminance.x, sceneDesc.sunIlluminance.y, sceneDesc.sunIlluminance.z);
	Raylib_SetSunDirection(scene, sceneDesc.sunDirection.x, sceneDesc.sunDirection.y, sceneDesc.sunDirection.z);
	Raylib_FinalizeScene(scene);

	CameraHandle camera = Raylib_CreateCamera();
	Raylib_CameraSetPosition(camera, cameraLocation.x, cameraLocation.y, cameraLocation.z);
	Raylib_CameraSetLookAt(camera, cameraLookAt.x, cameraLookAt.y, cameraLookAt.z);
	Raylib_CameraSetPerspective(camera, sceneDesc.fovY, settings.getViewportAspectWH());
	Raylib_CameraSetLens(camera, CAMERA_APERTURE, focalDistance);
	Raylib_CameraSetMotion(camera, CAMERA_BEGIN_CAPTURE, CAMERA_END_CAPTURE);

	// Render default image
	ImageHandle mainImage = Raylib_CreateImage(viewportWidth, viewportHeight);

	Raylib_Render(&settings, scene, camera, mainImage);

	if (Raylib_IsDenoiserSupported()
		&& bRunDenoiser
		&& settings.renderMode == RAYLIB_RENDERMODE_Default)
	{
		LOG("Run denoiser");

		// Render aux images
		ImageHandle wNormalImage = Raylib_CreateImage(viewportWidth, viewportHeight);
		ImageHandle albedoImage = Raylib_CreateImage(viewportWidth, viewportHeight);

		CameraHandle debugCamera = Raylib_CreateCamera();
		Raylib_CameraCopy((CameraHandle)camera, debugCamera);
		Raylib_CameraSetLens(debugCamera, 0.0f, focalDistance);

		RendererSettings debugSettings = settings;
		debugSettings.renderMode = RAYLIB_RENDERMODE_Albedo;
		Raylib_Render(&debugSettings, scene, debugCamera, albedoImage);
		debugSettings.renderMode = RAYLIB_RENDERMODE_MicrosurfaceNormal;
		Raylib_Render(&debugSettings, scene, debugCamera, wNormalImage);

		std::string albedoFilenameJPG = makeFilename("_0.jpg");
		std::string normalFilenameJPG = makeFilename("_1.jpg");
		Raylib_WriteImageToDisk(albedoImage, albedoFilenameJPG.c_str(), RAYLIB_IMAGEFILETYPE_Jpg);
		Raylib_WriteImageToDisk(wNormalImage, normalFilenameJPG.c_str(), RAYLIB_IMAGEFILETYPE_Jpg);
		LOG("Write aux albedo image to: %s", albedoFilenameJPG.c_str());
		LOG("Write aux normal image to: %s", normalFilenameJPG.c_str());

		ImageHandle denoisedOutput = Raylib_CreateImage(0, 0);
		Raylib_Denoise(
			mainImage,
			true, // HDR
			albedoImage,
			wNormalImage,
			denoisedOutput);
		Raylib_PostProcess(denoisedOutput);

		std::string denoiseFilenameJPG = makeFilename("_2.jpg");
		Raylib_WriteImageToDisk(denoisedOutput, denoiseFilenameJPG.c_str(), RAYLIB_IMAGEFILETYPE_Jpg);
		LOG("Write denoised image to: %s", denoiseFilenameJPG.c_str());

		Raylib_DestroyCamera(debugCamera);
		Raylib_DestroyImage(wNormalImage);
		Raylib_DestroyImage(albedoImage);
		Raylib_DestroyImage(denoisedOutput);
	}

	// Apply postprocessing after denoising
	Raylib_PostProcess(mainImage);
	
	std::string resultFilenameBMP = makeFilename(".bmp");
	std::string resultFilenameJPG = makeFilename(".jpg");
	Raylib_WriteImageToDisk(mainImage, resultFilenameBMP.c_str(), RAYLIB_IMAGEFILETYPE_Bitmap);
	Raylib_WriteImageToDisk(mainImage, resultFilenameJPG.c_str(), RAYLIB_IMAGEFILETYPE_Jpg);
	LOG("Write main image to: %s", resultFilenameBMP.c_str());
	LOG("Write main image to: %s", resultFilenameJPG.c_str());

	Raylib_DestroyScene(scene);
	Raylib_DestroyCamera(camera);
	Raylib_DestroyImage(mainImage);

	LOG("=== Rendering has completed ===");
	Raylib_FlushLogThread();
}

//////////////////////////////////////////////////////////////////////////
// Demo scene generator functions

using OBJTransformer = std::function<void(OBJModelHandle inModel)>;
bool GetOrCreateOBJ(
	const char* filename,
	OBJModelHandle& outModel,
	OBJTransformer transformer = nullptr)
{
	std::string fullpath = ResourceFinder::Get().Find(filename);

	OBJModelHandle model = g_objContainer.find(filename);
	if (model != NULL)
	{
		outModel = model;
		return true;
	}

	model = Raylib_LoadOBJModel(fullpath.c_str());
	if (model != NULL)
	{
		if (transformer)
		{
			transformer(model);
		}
		Raylib_FinalizeOBJModel(model);
		g_objContainer.insert(filename, model);
		outModel = model;
		return true;
	}
	model = outModel = NULL;
	return false;
}

SceneHandle CreateScene_CornellBox() {
	SCOPED_CPU_COUNTER(CreateScene_CornellBox);

	SceneHandle scene = Raylib_CreateScene();

	OBJModelHandle objModel;
	if (GetOrCreateOBJ("content/cornell_box/CornellBox-Mirror.obj", objModel))
	{
		Raylib_AddOBJModelToScene(scene, objModel);
	}

	return scene;
}

SceneHandle CreateScene_CarShowRoom()
{
	SCOPED_CPU_COUNTER(CreateScene_CarShowRoom);

	SceneHandle scene = Raylib_CreateScene();

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
	Raylib_AddSceneElement(scene, (SceneElementHandle)pillar);
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

	auto sphere0 = new Sphere(vec3(3.0f, 1.0f, 0.0f), 1.0f, new Lambertian(vec3(0.9f, 0.2f, 0.2f)));
	auto sphere1 = new Sphere(vec3(-3.0f, 1.0f, 0.0f), 1.0f, new Lambertian(vec3(0.2f, 0.9f, 0.2f)));
	auto sphere2 = new Sphere(vec3(0.0f, 1.0f, 3.0f), 1.0f, new Lambertian(vec3(0.2f, 0.2f, 0.9f)));
	Raylib_AddSceneElement(scene, (SceneElementHandle)sphere0);
	Raylib_AddSceneElement(scene, (SceneElementHandle)sphere1);
	Raylib_AddSceneElement(scene, (SceneElementHandle)sphere2);

	// Ground
	Sphere* ground = new Sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new Lambertian(vec3(0.5f, 0.5f, 0.5f)));
	Raylib_AddSceneElement(scene, (SceneElementHandle)ground);

	return scene;
}

SceneHandle CreateScene_BreakfastRoom()
{
	SCOPED_CPU_COUNTER(CreateScene_BreakfastRoom);

	SceneHandle scene = Raylib_CreateScene();

	OBJModelHandle objModel;
	if (GetOrCreateOBJ("content/breakfast_room/breakfast_room.obj", objModel))
	{
		Raylib_AddOBJModelToScene(scene, objModel);
	}

	return scene;
}

SceneHandle CreateScene_DabrovicSponza()
{
	SCOPED_CPU_COUNTER(CreateScene_DabrovicSponza);

	SceneHandle scene = Raylib_CreateScene();

	OBJModelHandle objModel;
	if (GetOrCreateOBJ("content/dabrovic_sponza/sponza.obj", objModel))
	{
		Raylib_AddOBJModelToScene(scene, objModel);
	}

	return scene;
}

SceneHandle CreateScene_FireplaceRoom()
{
	SCOPED_CPU_COUNTER(CreateScene_FireplaceRoom);

	SceneHandle scene = Raylib_CreateScene();

	OBJModelHandle objModel;
	if (GetOrCreateOBJ("content/fireplace_room/fireplace_room.obj", objModel))
	{
		Raylib_AddOBJModelToScene(scene, objModel);
	}

	return scene;
}

SceneHandle CreateScene_LivingRoom()
{
	SCOPED_CPU_COUNTER(CreateScene_LivingRoom);

	SceneHandle scene = Raylib_CreateScene();

	OBJModelHandle objModel;
	if (GetOrCreateOBJ("content/living_room/living_room.obj", objModel))
	{
		Raylib_AddOBJModelToScene(scene, objModel);
	}

	return scene;
}

SceneHandle CreateScene_SibenikCathedral()
{
	SCOPED_CPU_COUNTER(CreateScene_SibenikCathedral);

	SceneHandle scene = Raylib_CreateScene();

	OBJModelHandle objModel;
	if (GetOrCreateOBJ("content/sibenik/sibenik.obj", objModel))
	{
		Raylib_AddOBJModelToScene(scene, objModel);
	}

	return scene;
}

SceneHandle CreateScene_SanMiguel()
{
	SCOPED_CPU_COUNTER(CreateScene_SanMiguel);

	SceneHandle scene = Raylib_CreateScene();

	OBJModelHandle objModel;
	if (GetOrCreateOBJ("content/San_Miguel/san-miguel.obj", objModel))
	{
		Raylib_AddOBJModelToScene(scene, objModel);
	}

	return scene;
}

SceneHandle CreateScene_ObjModel()
{
	SCOPED_CPU_COUNTER(CreateScene_ObjModel);

#define OBJTEST_LOCAL_LIGHTS            1
#define OBJTEST_INCLUDE_TOADTTE         1
#define OBJTEST_INCLUDE_CUBE            1

	SceneHandle scene = Raylib_CreateScene();

	// Light source
#if OBJTEST_LOCAL_LIGHTS
	Material* M_pointLight0 = new DiffuseLight(vec3(5.0f, 0.0f, 0.0f));
	Material* M_pointLight1 = new DiffuseLight(vec3(0.0f, 4.0f, 5.0f));
	auto light0 = new Sphere(vec3(2.0f, 2.0f, 0.0f), 0.5f, M_pointLight0);
	auto light1 = new Sphere(vec3(-1.0f, 2.0f, 1.0f), 0.3f, M_pointLight1);
	Raylib_AddSceneElement(scene, (SceneElementHandle)light0);
	Raylib_AddSceneElement(scene, (SceneElementHandle)light1);
#endif

#if OBJTEST_INCLUDE_TOADTTE
	auto transformer = [](OBJModelHandle inModel){
		Raylib_TransformOBJModel(
			inModel,
			0.0f, 0.0f, 0.0f,
			-10.0f, 0.0f, 0.0f,
			0.07f, 0.07f, 0.07f
		);
	};
	OBJModelHandle model;
	if (GetOrCreateOBJ("content/Toadette/Toadette.obj", model, transformer))
	{
		Raylib_AddOBJModelToScene(scene, (OBJModelHandle)model);
	}
#endif

#if OBJTEST_INCLUDE_CUBE
	Material* M_cube0 = new Lambertian(vec3(0.9f, 0.1f, 0.1f));
	Material* M_cube1 = new Lambertian(vec3(0.1f, 0.1f, 0.9f));
	auto cube0 = new Cube(vec3(-4.0f, 0.3f, 0.0f), vec3(-3.0f, 0.5f, 1.0f), CAMERA_BEGIN_CAPTURE, vec3(0.0f, 0.05f, 0.0f), M_cube0);
	auto cube1 = new Cube(vec3(-5.5f, 0.0f, 0.0f), vec3(-4.5f, 2.0f, 2.0f), CAMERA_BEGIN_CAPTURE, vec3(0.0f, 0.05f, 0.0f), M_cube1);
	Raylib_AddSceneElement(scene, (SceneElementHandle)cube0);
	Raylib_AddSceneElement(scene, (SceneElementHandle)cube1);
#endif

	// #todo-raylib: How to deal with ImageHandle and shared_ptr<Image2D>
#if 0
	std::string imgpath = ResourceFinder::Get().Find("content/Toadette/Toadette_body.png");
	ImageHandle img = Raylib_LoadImage(imgpath.c_str());
	if (img != NULL)
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
#endif

	const int32 numFans = 8;
	const float fanAngle = 1.0f / (float)(numFans + 1);
	for (int32 i = 0; i <= numFans; ++i)
	{
		float fanBegin = (float)M_PI * fanAngle * i;
		float fanEnd = (float)M_PI * fanAngle * (i + 1);
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
		mat->SetRoughnessFallback(0.9f);
		//mat->SetMetallicFallback(1.0f);
#endif

		auto tri = new Triangle(v0, v1, v2, n, n, n, mat);
		Raylib_AddSceneElement(scene, (SceneElementHandle)tri);
	}

	// Ground
	auto ground = new Sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new Lambertian(vec3(0.5f, 0.5f, 0.5f)));
	Raylib_AddSceneElement(scene, (SceneElementHandle)ground);

	return scene;
}

SceneHandle CreateScene_RandomSpheres()
{
	SCOPED_CPU_COUNTER(CreateScene_RandomSpheres);

	SceneHandle scene = Raylib_CreateScene();

	auto ground = new Sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new Lambertian(vec3(0.5f, 0.5f, 0.5f)));
	Raylib_AddSceneElement(scene, (SceneElementHandle)ground);

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
					auto sphere = new Sphere(center, 0.2f,
						new Lambertian(vec3(Random() * Random(), Random() * Random(), Random() * Random())));
					Raylib_AddSceneElement(scene, (SceneElementHandle)sphere);
				}
				else if(choose_material < 0.95f)
				{
					auto sphere = new Sphere(center, 0.2f,
						new Metal(vec3(0.5f * (1.0f + Random()), 0.5f * (1.0f + Random()), 0.5f * (1.0f + Random())), 0.5f * Random()));
					Raylib_AddSceneElement(scene, (SceneElementHandle)sphere);
				}
				else
				{
					auto sphere = new Sphere(center, 0.2f, new Dielectric(1.5f));
					Raylib_AddSceneElement(scene, (SceneElementHandle)sphere);
				}
			}
		}
	}
	auto bigSphere0 = new Sphere(vec3(0.0f, 1.0f, 0.0f), 1.0f, new Dielectric(1.5f));
	auto bigSphere1 = new Sphere(vec3(-2.0f, 1.0f, 0.0f), 1.0f, new Lambertian(vec3(0.4f, 0.2f, 0.1f)));
	auto bigSphere2 = new Sphere(vec3(2.0f, 1.0f, 0.0f), 1.0f, new Metal(vec3(0.7f, 0.6f, 0.5f), 0.0f));
	Raylib_AddSceneElement(scene, (SceneElementHandle)bigSphere0);
	Raylib_AddSceneElement(scene, (SceneElementHandle)bigSphere1);
	Raylib_AddSceneElement(scene, (SceneElementHandle)bigSphere2);

	return scene;
}

SceneHandle CreateScene_FourSpheres()
{
	SCOPED_CPU_COUNTER(CreateScene_FourSpheres);

	SceneHandle scene = Raylib_CreateScene();

	float groundRoughness = 0.0f;
	Material* M_ground = MicrofacetMaterial::FromConstants(
		vec3(1.0f), groundRoughness, 0.0f, vec3(0.0f));

	Material* M_left = new Dielectric(1.0f, vec3(1.0f, 0.5f, 0.5f));
	Material* M_center = new Lambertian(vec3(0.8f, 0.3f, 0.3f));
	Material* M_right = new Metal(vec3(0.8f, 0.6f, 0.2f), 0.0f);

	auto sphere0 = new Sphere(vec3(0.0f, -100.5f, -1.0f), 100.0f, M_ground);
	auto sphere1 = new Sphere(vec3(-1.0f, 0.0f, -1.0f), 0.5f, M_left);
	auto sphere2 = new Sphere(vec3(0.0f, 0.0f, -1.0f), 0.5f, M_center);
	auto sphere3 = new Sphere(vec3(1.0f, 0.0f, -1.0f), 0.5f, M_right);
	Raylib_AddSceneElement(scene, (SceneElementHandle)sphere0);
	Raylib_AddSceneElement(scene, (SceneElementHandle)sphere1);
	Raylib_AddSceneElement(scene, (SceneElementHandle)sphere2);
	Raylib_AddSceneElement(scene, (SceneElementHandle)sphere3);

	return scene;
}
