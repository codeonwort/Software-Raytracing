#include "image.h"
#include "file.h"
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

#include <vector>


// Test scene settings
#define CREATE_RANDOM_SCENE     CreateScene_ObjModel
// TODO: BVH for static mesh is still too slow for bedroom
//#define CREATE_RANDOM_SCENE     CreateScene_Bedroom
#define CAMERA_LOCATION         vec3(3.0f, 1.0f, 3.0f)
#define CAMERA_LOOKAT           vec3(0.0f, 1.0f, -1.0f)
#define CAMERA_UP               vec3(0.0f, 1.0f, 0.0f)
#define CAMERA_APERTURE         0.01f
#define CAMERA_BEGIN_CAPTURE    0.0f
#define CAMERA_END_CAPTURE      5.0f
#define FOV_Y                   45.0f
#define VIEWPORT_WIDTH          1024
#define VIEWPORT_HEIGHT         512

// Debug configuration (features under development)
#define FAKE_SKY_LIGHT          1
#define LOCAL_LIGHTS            1
#define INCLUDE_TOADTTE         1
#define INCLUDE_CUBE            1
#define TEST_TEXTURE_MAPPING    1
#define TEST_IMAGE_LOADER       0
#define RESULT_FILENAME         SOLUTION_DIR "test.bmp"

// Rendering configuration
#define SAMPLES_PER_PIXEL       100
#define MAX_RECURSION           5
#define RAY_T_MIN               0.001f

// TODO: ResourceFinder can't find content/ in the project directory (.exe is generated in out/build/{Configuration}/src)
HitableList* CreateScene_Bedroom()
{
	SCOPED_CPU_COUNTER(CreateScene_Bedroom);

	std::vector<Hitable*> list;

	OBJModel bedroomModel;
	if (OBJLoader::SyncLoad("content/bedroom/iscv2.obj", bedroomModel))
	{
		Transform transform;
		transform.Init(vec3(0.0f, 0.0f, 0.0f), Rotator(-90.0f, 0.0f, 0.0f), vec3(0.1f, 0.1f, 0.1f));
		bedroomModel.staticMesh->ApplyTransform(transform);
		bedroomModel.staticMesh->Finalize();
		list.push_back(bedroomModel.staticMesh);
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
		model.staticMesh->ApplyTransform(transform);
		model.staticMesh->Finalize();
		list.push_back(model.staticMesh);
	}
#endif

#if INCLUDE_CUBE
	Material* cube_mat = new Lambertian(vec3(0.9f, 0.1f, 0.1f));
	Material* cube_mat2 = new Lambertian(vec3(0.1f, 0.1f, 0.9f));
	list.push_back(new Cube(vec3(-4.0f, 0.3f, 0.0f), vec3(-3.0f, 0.5f, 1.0f), CAMERA_BEGIN_CAPTURE, vec3(0.0f, 0.05f, 0.0f), cube_mat));
	list.push_back(new Cube(vec3(-5.5f, 0.0f, 0.0f), vec3(-4.5f, 2.0f, 2.0f), CAMERA_BEGIN_CAPTURE, vec3(0.0f, 0.05f, 0.0f), cube_mat2));
#endif

#if TEST_TEXTURE_MAPPING
	Image2D img;
	if (ImageLoader::SyncLoad("content/Toadette/Toadette_body.png", img))
	{
		PBRMaterial* pbr_mat = new PBRMaterial;
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
		float fanBegin = pi<float>* fanAngle* i;
		float fanEnd = pi<float> * fanAngle * (i + 1);
		float fanRadius = 2.0f + 1.0f * (float)i / numFans;
		float z = -2.0f;
		vec3 v0(fanRadius * std::cos(fanBegin), fanRadius * std::sin(fanBegin), z);
		vec3 v1(0.0f, 0.0f, z);
		vec3 v2(fanRadius * std::cos(fanEnd), fanRadius * std::sin(fanEnd), z);
		vec3 n(0.0f, 0.0f, 1.0f);
		vec3 color = vec3(0.3f, 0.3f, 0.3f) + 0.7f * RandomInHemisphere(vec3(0.0f, 0.0f, 1.0f));
		list.push_back(new Triangle(v0, v1, v2, n, n, n, new Lambertian(color)));
	}
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

void InitializeSubsystems()
{
	SCOPED_CPU_COUNTER(InitializeSubsystems);

	StartLogThread();
	ResourceFinder::Get().AddDirectory("./content/");
	ImageLoader::Initialize();
	OBJLoader::Initialize();
}
void DestroySubsystems()
{
	OBJLoader::Destroy();
	ImageLoader::Destroy();
	WaitForLogThread();
}

int main(int argc, char** argv)
{
	SCOPED_CPU_COUNTER(main);

	InitializeSubsystems();

	log("raytracing study");

#if TEST_IMAGE_LOADER
	Image2D test;
	if (ImageLoader::SyncLoad("content/odyssey.jpg", test))
	{
		WriteBitmap(test, RESULT_FILENAME);
		return 0;
	}
#endif

	const int32 width = VIEWPORT_WIDTH;
	const int32 height = VIEWPORT_HEIGHT;
	Image2D image(width, height, 0x123456);

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
		FOV_Y, (float)width/(float)height,
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
		rendererSettings.fakeSkyLight = FAKE_SKY_LIGHT;
	}
	Renderer renderer;
	// NOTE: This is a blocking operation for now.
	renderer.RenderScene(rendererSettings, worldBVH, &camera, &image);

	image.PostProcess();

	WriteBitmap(image, RESULT_FILENAME);

	log("image has been written as bitmap");

	//////////////////////////////////////////////////////////////////////////
	// Cleanup
	DestroySubsystems();

	return 0;
}
