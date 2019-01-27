#include "image.h"
#include "file.h"
#include "log.h"
#include "ray.h"
#include "sphere.h"
#include "camera.h"
#include "random.h"
#include "material.h"
#include <vector>

#define ANTI_ALIASING    1
#define NUM_SAMPLES      100 // Valid only if ANTI_ALISING == 1
#define GAMMA_CORRECTION 1

vec3 Scene(const ray& r, Hitable* world, int depth)
{
	HitResult result;
	if(world->Hit(r, 0.001f, FLOAT_MAX, result))
	{
		ray scattered;
		vec3 attenuation;
		if(depth < 50 && result.material->Scatter(r, result, attenuation, scattered))
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


int main(int argc, char** argv)
{
	log("raytracing study");

	const int32 width = 1024;
	const int32 height = 512;
	HDRImage image(width, height, 0x123456);

	log("generate a test image (width: %d, height: %d)", width, height);

	// Generate an image
	std::vector<Hitable*> list;
	list.push_back(new sphere(vec3(0.0f, 0.0f, -1.0f),    0.5f,   new Lambertian(vec3(0.8f, 0.3f, 0.3f))  ));
	list.push_back(new sphere(vec3(0.0f, -100.5f, -1.0f), 100.0f, new Lambertian(vec3(0.8f, 0.8f, 0.0f))  ));
	list.push_back(new sphere(vec3(1.0f, 0.0f, -1.0f),    0.5f,   new Metal(vec3(0.8f, 0.6f, 0.2f), 1.0f) ));
	//list.push_back(new sphere(vec3(-1.0f, 0.0f, -1.0f),   0.5f,   new Metal(vec3(0.8f, 0.8f, 0.8f), 0.3f) ));
	list.push_back(new sphere(vec3(-1.0f, 0.0f, -1.0f),   -0.5f,  new Dielectric(1.5f)                    ));
	list.push_back(new sphere(vec3(-1.0f, 0.0f, -1.0f),   -0.45f, new Dielectric(1.5f)                    ));
	Hitable* world = new HitableList(list.data(), list.size());

	Camera camera;

#if ANTI_ALIASING
	RNG randoms(4096 * 8);
#endif

	int32 milestoneIx = 0;
	std::vector<float> milestones(9);
	for(int32 i = 1 ; i <= 9; ++i)
	{
		milestones[i - 1] = (float)i / 10.0f;
	}

	for(int32 y = 0; y < height; ++y)
	{
		for(int32 x = 0; x < width; ++x)
		{
			vec3 accum;
#if ANTI_ALIASING
			for(int32 s = 0; s < NUM_SAMPLES; ++s)
			{
				float u = (float)x / (float)width;
				float v = (float)y / (float)height;
				u += randoms.Peek() / (float)width;
				v += randoms.Peek() / (float)height;
				ray r = camera.GetRay(u, v);
				vec3 scene = Scene(r, world);
				accum += scene;
			}
			accum /= (float)NUM_SAMPLES;
#else
			float u = (float)x / (float)width;
			float v = (float)y / (float)height;
			ray r = camera.GetRay(u, v);
			accum = Scene(r, world);
#endif

#if GAMMA_CORRECTION
			accum.x = pow(accum.x, 1.0f / 2.2f);
			accum.y = pow(accum.y, 1.0f / 2.2f);
			accum.z = pow(accum.z, 1.0f / 2.2f);
#endif

			Pixel px(accum.x, accum.y, accum.z);
			image.SetPixel(x, y, px);
		}

		float progress = (float)y / (float)height;
		if(milestoneIx < milestones.size() && progress >= milestones[milestoneIx])
		{
			log("%d percent complete...", (int32)(progress * 100));
			milestoneIx += 1;
		}
	}

	WriteBitmap(image, "test.bmp");

	log("image has been written as bitmap");

	return 0;
}

