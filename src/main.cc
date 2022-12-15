#include "image.h"
#include "log.h"
#include "camera.h"
#include "random.h"
#include "transform.h"
#include "renderer.h"
#include "util/stat.h"
#include "util/resource_finder.h"
#include "geom/bvh.h"
#include "geom/cube.h"
#include "geom/sphere.h"
#include "geom/triangle.h"
#include "geom/static_mesh.h"
#include "loader/obj_loader.h"
#include "loader/image_loader.h"
#include "shading/material.h"
#include "platform/platform.h"

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

// #todo: Wrap these with Scene = {objects,camera,viewport}
// 0: Cornell box
// 1: Demo scene with some primitives and a model
// 2: Car show room
// 3: Breakfast Room
// 4: Dabrovic Sponza
#define SCENE_CHOICE 3

#if SCENE_CHOICE == 0
	#define CREATE_RANDOM_SCENE     CreateScene_CornellBox
	#define CAMERA_LOCATION         vec3(0.0f, 1.0f, 4.0f)
	#define CAMERA_LOOKAT           vec3(0.0f, 1.0f, -1.0f)
	#define VIEWPORT_WIDTH          512
#elif SCENE_CHOICE == 1
	#define CREATE_RANDOM_SCENE     CreateScene_ObjModel
	#define CAMERA_LOCATION         vec3(3.0f, 1.0f, 3.0f)
	#define CAMERA_LOOKAT           vec3(0.0f, 1.0f, -1.0f)
#elif SCENE_CHOICE == 2
	#define CREATE_RANDOM_SCENE     CreateScene_CarShowRoom
	#define CAMERA_LOCATION         vec3(3.0f, 1.0f, 3.0f)
	#define CAMERA_LOOKAT           vec3(0.0f, 1.0f, -1.0f)
#elif SCENE_CHOICE == 3
	#define CREATE_RANDOM_SCENE     CreateScene_BreakfastRoom
	#define CAMERA_LOCATION         vec3(0.0f, 1.0f, 5.0f)
	#define CAMERA_LOOKAT           vec3(0.0f, 1.0f, -1.0f)
	#define FOV_Y                   60.0f
#elif SCENE_CHOICE == 4
	#define CREATE_RANDOM_SCENE     CreateScene_DabrovicSponza
	#define CAMERA_LOCATION         vec3(10.0f, 2.0f, 0.0f)
	#define CAMERA_LOOKAT           vec3(0.0f, 3.0f, 0.0f)
	#define FOV_Y                   60.0f
#elif SCENE_CHOICE == 5
	// #todo-obj: Support 1) alpha test for foliage, 2) other illumination models
	#define CREATE_RANDOM_SCENE     CreateScene_FireplaceRoom
	#define CAMERA_LOCATION         vec3(5.0f, 1.0f, -1.5f)
	#define CAMERA_LOOKAT           vec3(0.0f, 1.0f, -1.5f)
	#define FOV_Y                   60.0f
#elif SCENE_CHOICE == 6
	// #todo-obj: 7602 materials?????? There's only ~40 materials in living_room.mtl...
	#define CREATE_RANDOM_SCENE     CreateScene_LivingRoom
	#define CAMERA_LOCATION         vec3(1.0f, 2.0f, 0.0f)
	#define CAMERA_LOOKAT           vec3(0.0f, 2.0f, 0.0f)
	#define FOV_Y                   60.0f
#elif SCENE_CHOICE == 7
	// #todo-obj: Sky light cannnot penetrate opaque windows (need to handle translucency)
	#define CREATE_RANDOM_SCENE     CreateScene_SibenikCathedral
	#define CAMERA_LOCATION         vec3(-10.0f, -12.0f, 0.0f)
	#define CAMERA_LOOKAT           vec3(0.0f, -11.5f, 0.0f)
	#define FOV_Y                   60.0f
#elif SCENE_CHOICE == 8
	// #todo-obj: Again bloated number of materials.
	// Also not included in Setup.ps1 as this model is too big.
	#define CREATE_RANDOM_SCENE     CreateScene_SanMiguel
	#define CAMERA_LOCATION         vec3(0.0f, 0.0f, 0.0f)
	#define CAMERA_LOOKAT           vec3(0.0f, 0.0f, 1.0f)
	#define FOV_Y                   60.0f
#else
	#error Invalid value for SCENE_CHOICE
#endif

// Rendering configuration
#define CAMERA_UP               vec3(0.0f, 1.0f, 0.0f)
#define CAMERA_APERTURE         0.01f
#define CAMERA_BEGIN_CAPTURE    0.0f
#define CAMERA_END_CAPTURE      5.0f
#ifndef FOV_Y
	#define FOV_Y               45.0f
#endif
#ifndef VIEWPORT_WIDTH
	#define VIEWPORT_WIDTH      1024
#endif
#define VIEWPORT_HEIGHT         512

#define SAMPLES_PER_PIXEL       200
#define MAX_RECURSION           5
#define RAY_T_MIN               0.0001f
#define RENDERER_DEBUG_MODE     EDebugMode::None

vec3 FAKE_SKY_LIGHT(const vec3& dir)
{
	float t = 0.5f * (dir.y + 1.0f);
	return 3.0f * ((1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f));
}

// #todo-wip: Magic values
// Only for CreateScene_ObjModel()
#define LOCAL_LIGHTS            1
#define INCLUDE_TOADTTE         1
#define INCLUDE_CUBE            1
// Output file names
#define RESULT_FILENAME_BMP     SOLUTION_DIR "test.bmp"
#define RESULT_FILENAME_JPG     SOLUTION_DIR "test.jpg"
#define ALBEDO_FILENAME_JPG     SOLUTION_DIR "test_0.jpg"
#define NORMAL_FILENAME_JPG     SOLUTION_DIR "test_1.jpg"
#define DENOISE_FILENAME_JPG    SOLUTION_DIR "test_2.jpg"

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

// #todo-wip: Run a (input command -> execute command) loop.
int main(int argc, char** argv) {
	SCOPED_CPU_COUNTER(main);

	//
	// Initialize
	//
	InitializeSubsystems();

	log("raytracing study");

	const int32 width = VIEWPORT_WIDTH;
	const int32 height = VIEWPORT_HEIGHT;
	Image2D image(width, height, 0x0);

	log("generate a test image (width: %d, height: %d)", width, height);

	//
	// Create a scene
	//
	BVHNode* worldBVH = nullptr;
	{
		HitableList* world = CREATE_RANDOM_SCENE();
		worldBVH = new BVHNode(world, CAMERA_BEGIN_CAPTURE, CAMERA_END_CAPTURE);
	}

	const float dist_to_focus = (CAMERA_LOCATION - CAMERA_LOOKAT).Length();
	Camera camera(
		CAMERA_LOCATION, CAMERA_LOOKAT, CAMERA_UP,
		FOV_Y, (float)width / (float)height,
		CAMERA_APERTURE, dist_to_focus,
		CAMERA_BEGIN_CAPTURE, CAMERA_END_CAPTURE);

	//
	// Generate an image with multi-threading
	//
	RendererSettings rendererSettings;
	{
		rendererSettings.samplesPerPixel = SAMPLES_PER_PIXEL;
		rendererSettings.maxPathLength = MAX_RECURSION;
		rendererSettings.rayTMin = RAY_T_MIN;
		rendererSettings.skyLightFn = FAKE_SKY_LIGHT;
		rendererSettings.debugMode = RENDERER_DEBUG_MODE;
	}
	Renderer renderer;
	// NOTE: This is a blocking operation for now.
	renderer.RenderScene(rendererSettings, worldBVH, &camera, &image);

#if INTEL_DENOISER
	//
	// Generate aux output
	//
	Image2D imageWorldNormal(width, height, 0x0);
	Image2D imageAlbedo(width, height, 0x0);

	Camera debugCamera(
		CAMERA_LOCATION, CAMERA_LOOKAT, CAMERA_UP,
		FOV_Y, (float)width / (float)height,
		0.0f, dist_to_focus,
		CAMERA_BEGIN_CAPTURE, CAMERA_END_CAPTURE);

	rendererSettings.debugMode = EDebugMode::Reflectance;
	renderer.RenderScene(rendererSettings, worldBVH, &debugCamera, &imageAlbedo);
	rendererSettings.debugMode = EDebugMode::VertexNormal;
	renderer.RenderScene(rendererSettings, worldBVH, &debugCamera, &imageWorldNormal);

	WriteImageToDisk(imageAlbedo, ALBEDO_FILENAME_JPG, EImageFileType::Jpg);
	WriteImageToDisk(imageWorldNormal, NORMAL_FILENAME_JPG, EImageFileType::Jpg);

	// Dump as float array to pass to oidn
	std::vector<float> noisyInputBlob, albedoBlob, worldNormalBlob;
	image.DumpFloatRGBs(noisyInputBlob);
	imageAlbedo.DumpFloatRGBs(albedoBlob);
	imageWorldNormal.DumpFloatRGBs(worldNormalBlob);
#endif

	image.PostProcess();

	WriteImageToDisk(image, RESULT_FILENAME_BMP, EImageFileType::Bitmap);
	WriteImageToDisk(image, RESULT_FILENAME_JPG, EImageFileType::Jpg);

	log("image has been written as bitmap: %s", RESULT_FILENAME_BMP);
	log("image has been written as jpg: %s", RESULT_FILENAME_JPG);

#if INTEL_DENOISER
	{
		log("Denoise the result using Intel OpenImageDenoise");

		OIDNDevice device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);
		oidnCommitDevice(device);

		std::vector<float> denoisedOutputBlob(noisyInputBlob.size(), 0.0f);

		OIDNFilter filter = oidnNewFilter(device, "RT");
		oidnSetSharedFilterImage(filter, "color", noisyInputBlob.data(),
			OIDN_FORMAT_FLOAT3, width, height, 0, 0, 0); // noisy input
		oidnSetSharedFilterImage(filter, "albedo", albedoBlob.data(),
			OIDN_FORMAT_FLOAT3, width, height, 0, 0, 0); // noisy input
		oidnSetSharedFilterImage(filter, "normal", worldNormalBlob.data(),
			OIDN_FORMAT_FLOAT3, width, height, 0, 0, 0); // noisy input
		oidnSetSharedFilterImage(filter, "output", denoisedOutputBlob.data(),
			OIDN_FORMAT_FLOAT3, width, height, 0, 0, 0); // noisy input
		oidnSetFilter1b(filter, "hdr", true);
		oidnCommitFilter(filter);

		oidnExecuteFilter(filter);

		const char* err;
		if (oidnGetDeviceError(device, &err) != OIDN_ERROR_NONE)
		{
			log("oidn error: %s", err);
		}

		oidnReleaseFilter(filter);
		oidnReleaseDevice(device);

		Image2D denoisedOutput(width, height);
		size_t k = 0;
		for (uint32_t y = 0; y < height; ++y)
		{
			for (uint32_t x = 0; x < width; ++x)
			{
				Pixel px(denoisedOutputBlob[k], denoisedOutputBlob[k + 1], denoisedOutputBlob[k + 2]);
				denoisedOutput.SetPixel(x, y, px);
				k += 3;
			}
		}
		denoisedOutput.PostProcess();
		WriteImageToDisk(denoisedOutput, DENOISE_FILENAME_JPG, EImageFileType::Jpg);
	}
#endif

	//
	// Cleanup
	//
	DestroySubsystems();

	return 0;
}

HitableList* CreateScene_CornellBox() {
	SCOPED_CPU_COUNTER(CreateScene_CornellBox);

	std::vector<Hitable*> list;

	OBJModel objModel;
	if (OBJLoader::SyncLoad("content/cornell_box/CornellBox-Mirror.obj", objModel)) {
		objModel.FinalizeAllMeshes();
		list.push_back(objModel.rootObject);
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

	OBJModel objModel;
	if (OBJLoader::SyncLoad("content/breakfast_room/breakfast_room.obj", objModel))
	{
		Transform transform;
		transform.Init(vec3(0.0f), Rotator(0.0f, 0.0f, 0.0f), vec3(1.0f));

		std::for_each(objModel.staticMeshes.begin(), objModel.staticMeshes.end(),
			[&transform](StaticMesh* mesh) { mesh->ApplyTransform(transform); });
		objModel.FinalizeAllMeshes();

		list.push_back(objModel.rootObject);
	}

	return new HitableList(list);
}

HitableList* CreateScene_DabrovicSponza()
{
	SCOPED_CPU_COUNTER(CreateScene_DabrovicSponza);

	std::vector<Hitable*> list;

	OBJModel objModel;
	if (OBJLoader::SyncLoad("content/dabrovic_sponza/sponza.obj", objModel))
	{
		Transform transform;
		transform.Init(vec3(0.0f), Rotator(0.0f, 0.0f, 0.0f), vec3(1.0f));

		std::for_each(objModel.staticMeshes.begin(), objModel.staticMeshes.end(),
			[&transform](StaticMesh* mesh) { mesh->ApplyTransform(transform); });
		objModel.FinalizeAllMeshes();

		list.push_back(objModel.rootObject);
	}

	return new HitableList(list);
}


HitableList* CreateScene_FireplaceRoom()
{
	SCOPED_CPU_COUNTER(CreateScene_FireplaceRoom);

	std::vector<Hitable*> list;

	OBJModel objModel;
	if (OBJLoader::SyncLoad("content/fireplace_room/fireplace_room.obj", objModel))
	{
		Transform transform;
		transform.Init(vec3(0.0f), Rotator(0.0f, 0.0f, 0.0f), vec3(1.0f));

		std::for_each(objModel.staticMeshes.begin(), objModel.staticMeshes.end(),
			[&transform](StaticMesh* mesh) { mesh->ApplyTransform(transform); });
		objModel.FinalizeAllMeshes();

		list.push_back(objModel.rootObject);
	}

	return new HitableList(list);
}


HitableList* CreateScene_LivingRoom()
{
	SCOPED_CPU_COUNTER(CreateScene_LivingRoom);

	std::vector<Hitable*> list;

	OBJModel objModel;
	if (OBJLoader::SyncLoad("content/living_room/living_room.obj", objModel))
	{
		Transform transform;
		transform.Init(vec3(0.0f), Rotator(0.0f, 0.0f, 0.0f), vec3(1.0f));

		std::for_each(objModel.staticMeshes.begin(), objModel.staticMeshes.end(),
			[&transform](StaticMesh* mesh) { mesh->ApplyTransform(transform); });
		objModel.FinalizeAllMeshes();

		list.push_back(objModel.rootObject);
	}

	return new HitableList(list);
}


HitableList* CreateScene_SibenikCathedral()
{
	SCOPED_CPU_COUNTER(CreateScene_SibenikCathedral);

	std::vector<Hitable*> list;

	OBJModel objModel;
	if (OBJLoader::SyncLoad("content/sibenik/sibenik.obj", objModel))
	{
		Transform transform;
		transform.Init(vec3(0.0f), Rotator(0.0f, 0.0f, 0.0f), vec3(1.0f));

		std::for_each(objModel.staticMeshes.begin(), objModel.staticMeshes.end(),
			[&transform](StaticMesh* mesh) { mesh->ApplyTransform(transform); });
		objModel.FinalizeAllMeshes();

		list.push_back(objModel.rootObject);
	}

	return new HitableList(list);
}

HitableList* CreateScene_SanMiguel()
{
	SCOPED_CPU_COUNTER(CreateScene_SanMiguel);

	std::vector<Hitable*> list;

	OBJModel objModel;
	if (OBJLoader::SyncLoad("content/San_Miguel/san-miguel-low-poly.obj", objModel))
	{
		Transform transform;
		transform.Init(vec3(0.0f), Rotator(0.0f, 0.0f, 0.0f), vec3(1.0f));

		std::for_each(objModel.staticMeshes.begin(), objModel.staticMeshes.end(),
			[&transform](StaticMesh* mesh) { mesh->ApplyTransform(transform); });
		objModel.FinalizeAllMeshes();

		list.push_back(objModel.rootObject);
	}

	return new HitableList(list);
}

HitableList* CreateScene_ObjModel()
{
	SCOPED_CPU_COUNTER(CreateRandomScene);

	std::vector<Hitable*> list;

	// Light source
#if LOCAL_LIGHTS
	Material* pointLight0 = new DiffuseLight(vec3(5.0f, 0.0f, 0.0f));
	Material* pointLight1 = new DiffuseLight(vec3(0.0f, 4.0f, 5.0f));
	list.push_back(new sphere(vec3(2.0f, 2.0f, 0.0f), 0.5f, pointLight0));
	list.push_back(new sphere(vec3(-1.0f, 2.0f, 1.0f), 0.3f, pointLight1));
#endif

#if INCLUDE_TOADTTE
	OBJModel model;
	if (OBJLoader::SyncLoad("content/Toadette/Toadette.obj", model))
	{
		Transform transform;
		transform.Init(
			vec3(0.0f, 0.0f, 0.0f),
			Rotator(-10.0f, 0.0f, 0.0f),
			vec3(0.07f, 0.07f, 0.07f));
		std::for_each(model.staticMeshes.begin(), model.staticMeshes.end(),
			[&transform](StaticMesh* mesh) { mesh->ApplyTransform(transform); });
		model.FinalizeAllMeshes();
		list.push_back(model.rootObject);
	}
#endif

#if INCLUDE_CUBE
	Material* cube_mat = new Lambertian(vec3(0.9f, 0.1f, 0.1f));
	Material* cube_mat2 = new Lambertian(vec3(0.1f, 0.1f, 0.9f));
	list.push_back(new Cube(vec3(-4.0f, 0.3f, 0.0f), vec3(-3.0f, 0.5f, 1.0f), CAMERA_BEGIN_CAPTURE, vec3(0.0f, 0.05f, 0.0f), cube_mat));
	list.push_back(new Cube(vec3(-5.5f, 0.0f, 0.0f), vec3(-4.5f, 2.0f, 2.0f), CAMERA_BEGIN_CAPTURE, vec3(0.0f, 0.05f, 0.0f), cube_mat2));
#endif

	Image2D img;
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
	//float R = cos(pi<float> / 4.0f);
	std::vector<Hitable*> list;
	list.push_back(new sphere(vec3(0.0f, -100.5f, -1.0f), 100.0f, new Lambertian(vec3(0.8f, 0.8f, 0.0f))));
	list.push_back(new sphere(vec3(0.0f, 0.0f, -1.0f), 0.5f, new Lambertian(vec3(0.8f, 0.3f, 0.3f))));
	list.push_back(new sphere(vec3(1.0f, 0.0f, -1.0f), 0.5f, new Metal(vec3(0.8f, 0.6f, 0.2f), 1.0f)));
	list.push_back(new sphere(vec3(-1.0f, 0.0f, -1.0f), 0.5f, new Metal(vec3(0.8f, 0.8f, 0.8f), 0.3f)));
	//list.push_back(new sphere(vec3(-1.0f, 0.0f, -1.0f),   0.5f,  new Dielectric(1.5f)                    ));
	//list.push_back(new sphere(vec3(-1.0f, 0.0f, -1.0f),   -0.45f, new Dielectric(1.5f)                    ));
	HitableList* world = new HitableList(list);
	return world;
}
