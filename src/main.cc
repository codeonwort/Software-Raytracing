#include "image.h"
#include "file.h"
#include "log.h"
#include "ray.h"
#include "sphere.h"
#include "camera.h"
#include "random.h"
#include "material.h"
#include "thread_pool.h"
#include <vector>
#include <thread>


#define ANTI_ALIASING    0
#define NUM_SAMPLES      50 // Valid only if ANTI_ALISING == 1
#define GAMMA_CORRECTION 1
#define GAMMA_VALUE      2.2f
#define MAX_RECURSION    5

vec3 Scene(const ray& r, Hitable* world, int depth)
{
	HitResult result;
	if(world->Hit(r, 0.001f, FLOAT_MAX, result))
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

Hitable* CreateRandomScene()
{
	int n = 500;
	Hitable** list = new Hitable*[n+1];
	list[0] = (new sphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, new Lambertian(vec3(0.5f, 0.5f, 0.5f))));
	int32 i = 1;
	for(int32 a = -11; a < 11; ++a)
	{
		for(int32 b = -11; b < 11; ++b)
		{
			float choose_material = Random();
			vec3 center(a + 0.9f * Random(), 0.2f, b + 0.9f * Random());
			if((center - vec3(4.0f, 0.2f, 0.0f)).Length() > 2.0f)
			{
				if(choose_material < 0.8f)
				{
					list[i++] = new sphere(center, 0.2f,
						new Lambertian(vec3(Random()*Random(), Random()*Random(), Random()*Random())));
				}
				else if(choose_material < 0.95f)
				{
					list[i++] = new sphere(center, 0.2f,
						new Metal(vec3(0.5f * (1.0f + Random()), 0.5f * (1.0f + Random()), 0.5f * (1.0f + Random())), 0.5f * Random()));
				}
				else
				{
					list[i++] = new sphere(center, 0.2, new Dielectric(1.5f));
				}
			}
		}
	}
	list[i++] = new sphere(vec3(0.0f, 1.0f, 0.0f), 1.0f, new Dielectric(1.5f));
	list[i++] = new sphere(vec3(-2.0f, 1.0f, 0.0f), 1.0f, new Lambertian(vec3(0.4f, 0.2f, 0.1f)));
	list[i++] = new sphere(vec3(2.0f, 1.0f, 0.0f), 1.0f, new Metal(vec3(0.7f, 0.6f, 0.5f), 0.0f));

	return new HitableList(list, i);
}

#if ANTI_ALIASING
RNG randoms(4096 * 8);
#endif

struct WorkCell
{
	int32 x;
	int32 y;
	int32 width;
	int32 height;

	HDRImage* image;
	Camera* camera;
	Hitable* world;
};

void generateCell(const WorkItemParam* param)
{
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
				u += randoms.Peek() / imageWidth;
				v += randoms.Peek() / imageHeight;
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

int main(int argc, char** argv)
{

	log("raytracing study");

	const int32 width = 1024;
	const int32 height = 512;
	HDRImage image(width, height, 0x123456);

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
	Hitable* world = CreateRandomScene();
#endif

	vec3 camera_location(3.0f, 1.0f, 3.0f);
	vec3 camera_lookAt(0.0f, 1.0f, -1.0f);
	vec3 camera_up(0.0f, 1.0f, 0.0f);
	float dist_to_focus = (camera_location - camera_lookAt).Length();
	float aperture = 0.01f;
	Camera camera(
		camera_location, camera_lookAt, camera_up,
		45.0f, (float)width/(float)height,
		aperture, dist_to_focus);

	// Multi-threading
	uint32 numCores = std::max((uint32)1, (uint32)std::thread::hardware_concurrency());
	log("number of cores: %u", numCores);

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
		work.routine = generateCell;
		work.arg = &workCells[i];

		tp.AddWork(work);
	}

	log("number of work items: %d", (int32)workCells.size());

	constexpr bool blockingOperation = false;
	tp.Start(blockingOperation);

	// progress
	int32 milestoneIx = 0;
	std::vector<float> milestones(9);
	for(int32 i = 1 ; i <= 9; ++i)
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
	}

	WriteBitmap(image, "test.bmp");

	log("image has been written as bitmap");

	return 0;
}
