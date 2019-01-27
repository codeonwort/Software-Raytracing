#pragma once

#include "ray.h"
#include <limits>

#define FLOAT_MIN std::numeric_limits<float>::min()
#define FLOAT_MAX std::numeric_limits<float>::max()

struct HitResult
{
	float t; // ray.at(t) = p
	vec3  p; // position
	vec3  n; // normal
};

class Hitable
{

public:
	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& result) const = 0;	

};

class HitableList : public Hitable
{

public:
	HitableList() {}
	HitableList(Hitable** inList, int inNum)
		: list(inList)
		, n(inNum)
	{}

	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& result) const;

	Hitable** list;
	int n;
};

bool HitableList::Hit(const ray& r, float t_min, float t_max, HitResult& result) const
{
	HitResult temp;
	bool anyHit = false;
	float closest = t_max;
	for(int32 i = 0; i < n; ++i)
	{
		if(list[i]->Hit(r, t_min, closest, temp))
		{
			anyHit = true;
			closest = temp.t;
			result = temp;
		}
	}
	return anyHit;
}

