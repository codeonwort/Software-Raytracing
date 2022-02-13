#include "image.h"
#include "file.h"
#include "log.h"
#include "camera.h"
#include "random.h"
#include "material.h"
#include "transform.h"
#include "thread_pool.h"
#include "util/stat.h"
#include "util/resource_finder.h"
#include "geom/ray.h"
#include "geom/bvh.h"
#include "geom/cube.h"
#include "geom/sphere.h"
#include "geom/triangle.h"
#include "geom/static_mesh.h"
#include "loader/obj_loader.h"
#include "loader/image_loader.h"

#include <vector>
#include <thread>
#include <chrono>


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
#define FAKE_SKY_LIGHT          0
#define LOCAL_LIGHTS            1
#define BVH_FOR_SCENE           1
#define INCLUDE_TOADTTE         1
#define INCLUDE_CUBE            1
#define TEST_TEXTURE_MAPPING    1
#define TEST_IMAGE_LOADER       0
#define RESULT_FILENAME         "test.bmp"

// Rendering configuration
#define SAMPLES_PER_PIXEL       1000
#define MAX_RECURSION           5
#define RAY_T_MIN               0.001f

vec3 TraceScene(const ray& r, Hitable* world, int depth)
{
	HitResult result;
	if (world->Hit(r, RAY_T_MIN, FLOAT_MAX, result))
	{
		ray scattered;
		vec3 attenuation;
		vec3 emitted = result.material->Emitted(result.paramU, result.paramV, result.p);
		if (depth < MAX_RECURSION && result.material->Scatter(r, result, attenuation, scattered))
		{
			return emitted + attenuation * TraceScene(scattered, world, depth + 1);
		}
		else
		{
			return emitted;
		}
	}

	// #todo: Support sky cubemap
#if FAKE_SKY_LIGHT
	vec3 dir = r.d;
	dir.Normalize();
	float t = 0.5f * (dir.y + 1.0f);
	return 3.0f * ((1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f));
#else
	return vec3(0.0f, 0.0f, 0.0f);
#endif
}
vec3 TraceScene(const ray& r, Hitable* world)
{
	return TraceScene(r, world, 0);
}

// TODO: ResourceFinder can't find content/ in the project directory (.exe is generated in out/build/{Configuration}/src)
Hitable* CreateScene_Bedroom()
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

Hitable* CreateScene_ObjModel()
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
		TextureMaterial* tm = new TextureMaterial(img);
		const vec3 origin(1.0f, 0.0f, 0.0f);
		const vec3 n(0.0f, 0.0f, 1.0f);
 		{
			Triangle* T = new Triangle(
				origin + vec3(0.0f, 0.0f, 0.0f), origin + vec3(1.0f, 0.0f, 0.0f), origin + vec3(1.0f, 1.0f, 0.0f),
				n, n, n, tm);
 			T->SetParameterization(0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
			list.push_back(T);
 		}
 		{
 			Triangle* T = new Triangle(
				origin + vec3(0.0f, 0.0f, 0.0f), origin + vec3(1.0f, 1.0f, 0.0f), origin + vec3(0.0f, 1.0f, 0.0f),
				n, n, n, tm);
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
		vec3 color = RandomInUnitSphere();
		list.push_back(new Triangle(v0, v1, v2, n, n, n, new Lambertian(color)));
	}
	list.push_back(new sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new Lambertian(vec3(0.5f, 0.5f, 0.5f))));

	return new HitableList(list);
}

Hitable* CreateScene_RandomSpheres()
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

Hitable* CreateScene_FourSpheres()
{
	//float R = cos(pi<float> / 4.0f);
	std::vector<Hitable*> list;
	list.push_back(new sphere(vec3(0.0f, -100.5f, -1.0f), 100.0f, new Lambertian(vec3(0.8f, 0.8f, 0.0f))));
	list.push_back(new sphere(vec3(0.0f, 0.0f, -1.0f), 0.5f, new Lambertian(vec3(0.8f, 0.3f, 0.3f))));
	list.push_back(new sphere(vec3(1.0f, 0.0f, -1.0f), 0.5f, new Metal(vec3(0.8f, 0.6f, 0.2f), 1.0f)));
	list.push_back(new sphere(vec3(-1.0f, 0.0f, -1.0f), 0.5f, new Metal(vec3(0.8f, 0.8f, 0.8f), 0.3f)));
	//list.push_back(new sphere(vec3(-1.0f, 0.0f, -1.0f),   0.5f,  new Dielectric(1.5f)                    ));
	//list.push_back(new sphere(vec3(-1.0f, 0.0f, -1.0f),   -0.45f, new Dielectric(1.5f)                    ));
	Hitable* world = new HitableList(list);
	return world;
}

struct WorkCell
{
	int32 x;
	int32 y;
	int32 width;
	int32 height;

	Image2D* image;
	Camera* camera;
	Hitable* world;
};

void GenerateCell(const WorkItemParam* param)
{
	static thread_local RNG randomsAA(4096 * 8);

	int32 threadID = param->threadID;
	WorkCell* cell = reinterpret_cast<WorkCell*>(param->arg);

	const int32 endY = cell->y + cell->height;
	const int32 endX = cell->x + cell->width;

	const float imageWidth = (float)cell->image->GetWidth();
	const float imageHeight = (float)cell->image->GetHeight();

	const int32 SPP = std::max(1, SAMPLES_PER_PIXEL);
	for(int32 y = cell->y; y < endY; ++y)
	{
		for(int32 x = cell->x; x < endX; ++x)
		{
			vec3 accum(0.0f, 0.0f, 0.0f);
			for(int32 s = 0; s < SPP; ++s)
			{
				float u = (float)x / imageWidth;
				float v = (float)y / imageHeight;
				if (s != 0)
				{
					u += (randomsAA.Peek() - 0.5f) * 2.0f / imageWidth;
					v += (randomsAA.Peek() - 0.5f) * 2.0f / imageHeight;
				}
				ray r = cell->camera->GetRay(u, v);
				vec3 scene = TraceScene(r, cell->world);
				accum += scene;
			}
			accum /= (float)SPP;
			Pixel px(accum.x, accum.y, accum.z);
			cell->image->SetPixel(x, y, px);
		}
	}
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
	Hitable* world = CREATE_RANDOM_SCENE();
#if BVH_FOR_SCENE
	world = new BVHNode(static_cast<HitableList*>(world), CAMERA_BEGIN_CAPTURE, CAMERA_END_CAPTURE);
#endif

	const float dist_to_focus = (CAMERA_LOCATION - CAMERA_LOOKAT).Length();
	Camera camera(
		CAMERA_LOCATION, CAMERA_LOOKAT, CAMERA_UP,
		FOV_Y, (float)width/(float)height,
		CAMERA_APERTURE, dist_to_focus,
		CAMERA_BEGIN_CAPTURE, CAMERA_END_CAPTURE);

	//
	// Generate an image with multi-threading
	//
	uint32 numCores = std::max((uint32)1, (uint32)std::thread::hardware_concurrency());
	log("number of logical cores: %u", numCores);

	ThreadPool tp;
	tp.Initialize(numCores);

	std::vector<WorkCell> workCells;
	constexpr int32 cellWidth = 32;
	constexpr int32 cellHeight = 32;
	for(int32 x = 0; x < width; x += cellWidth)
	{
		for(int32 y = 0; y < height; y += cellHeight)
		{
			WorkCell cell;
			cell.x = x;
			cell.y = y;
			cell.width = std::min(cellWidth, width - x);
			cell.height = std::min(cellHeight, height - y);
			cell.image = &image;
			cell.camera = &camera;
			cell.world = world;
			workCells.emplace_back(cell);
		}
	}
	for(auto i=0u; i<workCells.size(); ++i)
	{
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
		for(int32 i = 1; i <= 9; ++i)
		{
			milestones[i - 1] = (float)i / 10.0f;
		}

		while (true)
		{
			if (tp.IsDone())
			{
				break;
			}
			else
			{
				float progress = tp.GetProgress();
				if (milestoneIx < milestones.size() && progress >= milestones[milestoneIx])
				{
					log("%d percent complete...", (int32)(progress * 100));
					milestoneIx += 1;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	image.PostProcess();

	WriteBitmap(image, RESULT_FILENAME);

	log("image has been written as bitmap");

	//////////////////////////////////////////////////////////////////////////
	// Cleanup
	DestroySubsystems();

	return 0;
}
