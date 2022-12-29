#include "cube.h"

bool Cube::Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const
{
	const vec3 movement = velocity * std::max(0.0f, r.t - timeStartMove);
	vec3 minBoundsT = minBounds + movement;
	vec3 maxBoundsT = maxBounds + movement;

	// https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
	float t[9];
	t[1] = (minBoundsT.x - r.o.x) / r.d.x;
	t[2] = (maxBoundsT.x - r.o.x) / r.d.x;
	t[3] = (minBoundsT.y - r.o.y) / r.d.y;
	t[4] = (maxBoundsT.y - r.o.y) / r.d.y;
	t[5] = (minBoundsT.z - r.o.z) / r.d.z;
	t[6] = (maxBoundsT.z - r.o.z) / r.d.z;
	t[7] = std::max(std::max(std::min(t[1], t[2]), std::min(t[3], t[4])), std::min(t[5], t[6]));
	t[8] = std::min(std::min(std::max(t[1], t[2]), std::max(t[3], t[4])), std::max(t[5], t[6]));

	if ((t[8] < 0 || t[7] > t[8]))
	{
		return false;
	}

	if (t_min <= t[7] && t[7] <= t_max)
	{
		outResult.material = material;
		outResult.p = r.at(t[7]);
		outResult.t = t[7];

		// #todo: Improve this stupid branching
		if (t[7] == t[1]) outResult.n = vec3(-1.0f, 0.0f, 0.0f);
		else if (t[7] == t[2]) outResult.n = vec3(1.0f, 0.0f, 0.0f);
		else if (t[7] == t[3]) outResult.n = vec3(0.0f, -1.0f, 0.0f);
		else if (t[7] == t[4]) outResult.n = vec3(0.0f, 1.0f, 0.0f);
		else if (t[7] == t[5]) outResult.n = vec3(0.0f, 0.0f, -1.0f);
		else if (t[7] == t[6]) outResult.n = vec3(0.0f, 0.0f, 1.0f);

		return true;
	}

	return false;
}

bool Cube::BoundingBox(float t0, float t1, AABB& outBox) const
{
	const vec3 movement0 = velocity * std::max(0.0f, t0 - timeStartMove);
	const vec3 movement1 = velocity * std::max(0.0f, t1 - timeStartMove);
	outBox = AABB(minBounds + movement0, maxBounds + movement0)
		+ AABB(minBounds + movement1, maxBounds + movement1);
	return true;
}
