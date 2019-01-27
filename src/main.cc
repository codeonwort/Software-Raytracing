#include "image.h"
#include "file.h"
#include "log.h"
#include "ray.h"
#include "sphere.h"
#include <vector>

vec3 Scene(const ray& r, Hitable* world)
{
	HitResult result;
	if(world->Hit(r, 0.0f, FLOAT_MAX, result))
	{
		return 0.5f * (result.n + 1.0f);
	}

	vec3 dir = r.d;
	dir.Normalize();
	float t = 0.5f * (dir.y + 1.0f);
	return (1.0f-t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);
}

int main(int argc, char** argv)
{
	log("raytracing study");

	log("generate a test image");

	const int32 width = 1024;
	const int32 height = 512;
	HDRImage image(width, height, 0x123456);

	// Generate an image
	vec3 top_left(-2.0f, 1.0f, -1.0f);
	vec3 horizontal(4.0f, 0.0f, 0.0f);
	vec3 vertical(0.0f, -2.0f, 0.0f);
	vec3 origin(0.0f, 0.0f, 0.0f);

	std::vector<Hitable*> list;
	list.push_back(new sphere(vec3(0.0f, 0.0f, -1.0f), 0.5f));
	list.push_back(new sphere(vec3(0.0f, -100.5f, -1.0f), 100.0f));
	Hitable* world = new HitableList(list.data(), list.size());

	for(int y = 0; y < height; ++y)
	{
		for(int x = 0; x < width; ++x)
		{
			float u = (float)x / float(width);
			float v = (float)y / float(height);
			ray r(origin, top_left + u * horizontal + v * vertical);
			vec3 scene = Scene(r, world);
			Pixel px(scene.x, scene.y, scene.z);
			image.SetPixel(x, y, px);
		}
	}

	WriteBitmap(image, "test.bmp");

	log("image has been written as bitmap");

	return 0;
}

