#pragma once

#include "ray.h"
#include "src/util/assertion.h"

#include <limits>
#include <vector>

#define FLOAT_MIN std::numeric_limits<float>::min()
#define FLOAT_MAX std::numeric_limits<float>::max()

class Material;

struct HitResult
{
	float     t; // ray.at(t) = p
	vec3      p; // position
	vec3      n; // normal
	Material* material;
};

class Hitable
{

public:
	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const = 0;	

};

class HitableList : public Hitable
{

public:
	HitableList() {}
	HitableList(std::vector<Hitable*> inList)
		: list(inList)
	{}

	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const;

	std::vector<Hitable*> list;
};
