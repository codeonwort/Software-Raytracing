#include "core/vec3.h"
#include "ray.h"

class AABB
{

public:
	AABB() : AABB(vec3(), vec3()) {}
	AABB(const vec3& inMin, const vec3& inMax)
		: minBounds(inMin)
		, maxBounds(inMax)
	{}

	bool Hit(const ray& r, float tMin, float tMax) const
	{
#if 0	// Copied from Cube::Hit()
		// https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
		float t[9];
		t[1] = (minBounds.x - r.o.x) / r.d.x;
		t[2] = (maxBounds.x - r.o.x) / r.d.x;
		t[3] = (minBounds.y - r.o.y) / r.d.y;
		t[4] = (maxBounds.y - r.o.y) / r.d.y;
		t[5] = (minBounds.z - r.o.z) / r.d.z;
		t[6] = (maxBounds.z - r.o.z) / r.d.z;
		t[7] = fmax(fmax(fmin(t[1], t[2]), fmin(t[3], t[4])), fmin(t[5], t[6]));
		t[8] = fmin(fmin(fmax(t[1], t[2]), fmax(t[3], t[4])), fmax(t[5], t[6]));

		if ((t[8] < 0 || t[7] > t[8]))
		{
			return false;
		}

		if (tMin <= t[7] && t[7] <= tMax)
		{
			return true;
		}

		return false;
#else
		// Ray Tracing in The Next Week
		for (int32 a = 0; a < 3; ++a)
		{
			float invD = 1.0f / r.d[a];
			float t0 = (minBounds[a] - r.o[a]) * invD;
			float t1 = (maxBounds[a] - r.o[a]) * invD;
			if (invD < 0.0f) std::swap(t0, t1);
			tMin = t0 > tMin ? t0 : tMin;
			tMax = t1 < tMax ? t1 : tMax;
			// #todo: 2D AABB can't pass this test (tMax == tMin)
			//if (tMax <= tMin) return false;
			if (tMax < tMin) return false;
		}
		return true;
#endif
	}

	vec3 minBounds;
	vec3 maxBounds;

};

inline AABB operator+(const AABB& a, const AABB& b)
{
	vec3 minBounds = min(a.minBounds, b.minBounds);
	vec3 maxBounds = max(a.maxBounds, b.maxBounds);
	return AABB(minBounds, maxBounds);
}
