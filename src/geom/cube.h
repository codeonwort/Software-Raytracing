#pragma once

#include "hit.h"

class Material;

class Cube : public Hitable
{

public:
	__forceinline static Cube FromMinMaxBounds(const vec3& inMinBounds, const vec3& inMaxBounds, float inTimeStartMove, vec3 inVelocity, Material* inMaterial)
	{
		return Cube(inMinBounds, inMaxBounds, inTimeStartMove, inVelocity, inMaterial);
	}
	__forceinline static Cube FromOriginAndExtent(const vec3& inOrigin, const vec3& inExtent, float inTimeStartMove, vec3 inVelocity, Material* inMaterial)
	{
		return Cube(inOrigin - inExtent, inOrigin + inExtent, inTimeStartMove, inVelocity, inMaterial);
	}

public:
	Cube(const vec3& inMinBounds, const vec3& inMaxBounds, float inTimeStartMove, vec3 inVelocity, Material* inMaterial)
		: minBounds(inMinBounds)
		, maxBounds(inMaxBounds)
		, timeStartMove(inTimeStartMove)
		, velocity(inVelocity)
		, material(inMaterial)
	{
	}

	Cube() : Cube(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 0.0f, vec3(0.0f, 0.0f, 0.0f), nullptr) {}

	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const
	{
		const vec3 movement = velocity * fmax(0.0f, r.t - timeStartMove);
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
		t[7] = fmax(fmax(fmin(t[1], t[2]), fmin(t[3], t[4])), fmin(t[5], t[6]));
		t[8] = fmin(fmin(fmax(t[1], t[2]), fmax(t[3], t[4])), fmax(t[5], t[6]));

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

	virtual bool BoundingBox(float t0, float t1, AABB& outBox) const override
	{
		const vec3 movement0 = velocity * fmax(0.0f, t0 - timeStartMove);
		const vec3 movement1 = velocity * fmax(0.0f, t1 - timeStartMove);
		outBox = AABB(minBounds + movement0, maxBounds + movement0)
			   + AABB(minBounds + movement1, maxBounds + movement1);
		return true;
	}

	/*
	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const
	{
		bool anyHit = false;
		vec3 n(0.0f, 0.0f, 0.0f);
		float t = -FLOAT_MAX;

		auto Check = [&](float t_cand, const vec3& n_cand) -> void
		{
			vec3 p_cand = r.at(t_cand);
			bool inside = minBounds.x <= p_cand.x && p_cand.x <= maxBounds.x
				&& minBounds.y <= p_cand.y && p_cand.y <= maxBounds.y
				&& minBounds.z <= p_cand.z && p_cand.z <= maxBounds.z;
			if (t_min <= t_cand && t_cand <= t_max && inside)
			{
				if (anyHit)
				{
					if (t_cand < t)
					{
						t = t_cand;
						n = n_cand;
					}
				}
				else
				{
					anyHit = true;
					t = t_cand;
					n = n_cand;
				}
			}
		};

		// yz-plane
		Check((minBounds.x - r.o.x) / r.d.x, vec3(-1.0f, 0.0f, 0.0f));
		Check((maxBounds.x - r.o.x) / r.d.x, vec3(1.0f, 0.0f, 0.0f));
		// zx-plane
		Check((minBounds.y - r.o.y) / r.d.y, vec3(0.0f, -1.0f, 0.0f));
		Check((maxBounds.y - r.o.y) / r.d.y, vec3(0.0f, 1.0f, 0.0f));
		// xy-plane
		Check((minBounds.z - r.o.z) / r.d.z, vec3(0.0f, 0.0f, -1.0f));
		Check((maxBounds.z - r.o.z) / r.d.z, vec3(0.0f, 0.0f, 1.0f));

		if (anyHit)
		{
			outResult.material = material;
			outResult.p = r.at(t);
			outResult.t = t;
			outResult.n = n;
		}

		return anyHit;
	}
	*/

	vec3 minBounds;
	vec3 maxBounds;
	float timeStartMove;
	vec3 velocity;
	Material* material;

};
