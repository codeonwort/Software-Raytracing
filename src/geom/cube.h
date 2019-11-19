#pragma once

#include "hit.h"
#include "src/material.h"

class Cube : public Hitable
{

public:
	__forceinline static Cube FromMinMaxBounds(const vec3& inMinBounds, const vec3& inMaxBounds, Material* inMaterial)
	{
		return Cube(inMinBounds, inMaxBounds, inMaterial);
	}
	__forceinline static Cube FromOriginAndExtent(const vec3& inOrigin, const vec3& inExtent, Material* inMaterial)
	{
		return Cube(inOrigin - inExtent, inOrigin + inExtent, inMaterial);
	}

public:
	Cube(const vec3& inMinBounds, const vec3& inMaxBounds, Material* inMaterial)
		: minBounds(inMinBounds)
		, maxBounds(inMaxBounds)
		, material(inMaterial)
	{
	}

	Cube() : Cube(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), nullptr) {}

	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const
	{
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
	Material* material;

};
