#include "image.h"
#include "image_loader.h"
#include "file.h"
#include "log.h"
#include "camera.h"
#include "random.h"
#include "material.h"
#include "transform.h"
#include "thread_pool.h"

#include <vector>
#include <thread>
#include <chrono>

// Test scene settings
#define CREATE_RANDOM_SCENE CreateRandomScene2
#define CAMERA_LOCATION     vec3(3.0f, 1.0f, 3.0f)
#define CAMERA_LOOKAT       vec3(0.0f, 1.0f, -1.0f)
#define CAMERA_UP           vec3(0.0f, 1.0f, 0.0f)

#define ANTI_ALIASING    1
#define NUM_SAMPLES      50 // Valid only if ANTI_ALISING == 1
#define GAMMA_CORRECTION 1
#define GAMMA_VALUE      2.2f
#define MAX_RECURSION    5
#define RAY_T_MIN        0.001f

vec3 Scene(const ray& r, Hitable* world, int depth)
{
	HitResult result;
	if(world->Hit(r, RAY_T_MIN, FLOAT_MAX, result))
	{
		ray scattered;
		vec3 attenuation;
		if(depth < MAX_RECURSION && result.material->Scatter(r, result, attenuation, scattered))
		{
			return attenuation * Scene(scattered, world, depth + 1);
		}
		else
		{
			return vec3(0.0f, 0.0f, 0.0f);
		}
	}

	vec3 dir = r.d;
	dir.Normalize();
	float t = 0.5f * (dir.y + 1.0f);
	return (1.0f-t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);
}
vec3 Scene(const ray& r, Hitable* world)
{
	return Scene(r, world, 0);
}

Hitable* CreateRandomScene2()
{
	std::vector<Hitable*> list;

#if 1 // OBJLoader test
	OBJModel model;
	if (OBJLoader::SyncLoad("content/Toadette/Toadette.obj", model))
	{
		Transform transform;
		transform.Init(vec3(0.0f, 0.0f, 0.0f), vec3(0.07f, 0.07f, 0.07f));
		model.staticMesh->ApplyTransform(transform);
		list.push_back(model.staticMesh);
	}
#endif

#if 0 // Texture mapping test
	Image2D img;
	if (ImageLoader::SyncLoad("content/Toadette/Toadette_body.png", img))
	{
		TextureMaterial* tm = new TextureMaterial(img);
 		{
 			Triangle* T = new Triangle(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f), tm);
 			T->SetParameterization(0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);
			list.push_back(T);
 		}
 		{
 			Triangle* T = new Triangle(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), tm);
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
		vec3 color = RandomInUnitSphere();
		list.push_back(new Triangle(v0, v1, v2, new Lambertian(color)));
	}
	list.push_back(new sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new Lambertian(vec3(0.5f, 0.5f, 0.5f))));

	return new HitableList(list);
}

Hitable* CreateRandomScene()
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
#if ANTI_ALIASING
	static thread_local RNG randomsAA(4096 * 8);
#endif

	int32 threadID = param->threadID;
	WorkCell* cell = reinterpret_cast<WorkCell*>(param->arg);

	const int32 endY = cell->y + cell->height;
	const int32 endX = cell->x + cell->width;

	const float imageWidth = (float)cell->image->GetWidth();
	const float imageHeight = (float)cell->image->GetHeight();

	for(int32 y = cell->y; y < endY; ++y)
	{
		for(int32 x = cell->x; x < endX; ++x)
		{
			vec3 accum;
#if ANTI_ALIASING
			for(int32 s = 0; s < NUM_SAMPLES; ++s)
			{
				float u = (float)x / imageWidth;
				float v = (float)y / imageHeight;
				u += randomsAA.Peek() / imageWidth;
				v += randomsAA.Peek() / imageHeight;
				ray r = cell->camera->GetRay(u, v);
				vec3 scene = Scene(r, cell->world);
				accum += scene;
			}
			accum /= (float)NUM_SAMPLES;
#else
			float u = (float)x / imageWidth;
			float v = (float)y / imageHeight;
			ray r = cell->camera->GetRay(u, v);
			accum = Scene(r, cell->world);
#endif

#if GAMMA_CORRECTION
			accum.x = pow(accum.x, 1.0f / GAMMA_VALUE);
			accum.y = pow(accum.y, 1.0f / GAMMA_VALUE);
			accum.z = pow(accum.z, 1.0f / GAMMA_VALUE);
#endif

			Pixel px(accum.x, accum.y, accum.z);
			cell->image->SetPixel(x, y, px);
		}
	}
}

void InitializeSubsystems()
{
	StartLogThread();
	ImageLoader::Initialize();
}
void DestroySubsystems()
{
	ImageLoader::Destroy();
}

int main(int argc, char** argv)
{
	InitializeSubsystems();

	log("raytracing study");

#if 0 // ImageLoader test
	Image2D test;
	if (ImageLoader::SyncLoad("content/odyssey.jpg", test))
	{
		WriteBitmap(test, "test.bmp");
		return 0;
	}
#endif

	const int32 width = 1024;
	const int32 height = 512;
	Image2D image(width, height, 0x123456);

	log("generate a test image (width: %d, height: %d)", width, height);

	// Generate an image
#if 0
	float R = cos(pi<float> / 4.0f);
	std::vector<Hitable*> list;
	list.push_back(new sphere(vec3(0.0f, 0.0f, -1.0f),    0.5f,   new Lambertian(vec3(0.8f, 0.3f, 0.3f))  ));
	list.push_back(new sphere(vec3(0.0f, -100.5f, -1.0f), 100.0f, new Lambertian(vec3(0.8f, 0.8f, 0.0f))  ));
	list.push_back(new sphere(vec3(1.0f, 0.0f, -1.0f),    0.5f,   new Metal(vec3(0.8f, 0.6f, 0.2f), 1.0f) ));
	//list.push_back(new sphere(vec3(-1.0f, 0.0f, -1.0f),   0.5f,   new Metal(vec3(0.8f, 0.8f, 0.8f), 0.3f) ));
	list.push_back(new sphere(vec3(-1.0f, 0.0f, -1.0f),   -0.5f,  new Dielectric(1.5f)                    ));
	list.push_back(new sphere(vec3(-1.0f, 0.0f, -1.0f),   -0.45f, new Dielectric(1.5f)                    ));
	Hitable* world = new HitableList(list.data(), list.size());
#else
	Hitable* world = CREATE_RANDOM_SCENE();
#endif

	float dist_to_focus = (CAMERA_LOCATION - CAMERA_LOOKAT).Length();
	float aperture = 0.01f;
	Camera camera(
		CAMERA_LOCATION, CAMERA_LOOKAT, CAMERA_UP,
		45.0f, (float)width/(float)height,
		aperture, dist_to_focus);

	// Multi-threading
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

	constexpr bool blockingOperation = false;
	tp.Start(blockingOperation);

	// progress
	int32 milestoneIx = 0;
	std::vector<float> milestones(9);
	for(int32 i = 1; i <= 9; ++i)
	{
		milestones[i - 1] = (float)i / 10.0f;
	}

	while(true)
	{
		if(tp.IsDone())
		{
			break;
		}
		else
		{
			float progress = tp.GetProgress();
			if(milestoneIx < milestones.size() && progress >= milestones[milestoneIx])
			{
				log("%d percent complete...", (int32)(progress * 100));
				milestoneIx += 1;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	WriteBitmap(image, "test.bmp");

	log("image has been written as bitmap");

	//////////////////////////////////////////////////////////////////////////
	// Cleanup
	DestroySubsystems();
	return 0;
}
