#pragma once

#include "ray.h"
#include "aabb.h"
#include "src/util/assertion.h"

#include <limits>
#include <vector>

#define FLOAT_MIN std::numeric_limits<float>::min()
#define FLOAT_MAX std::numeric_limits<float>::max()

class Material;

struct HitResult
{
	float     t; // ray.at(t) = p
	vec3      p; // position (world space)
	vec3      n; // normal (world space)

	// surface parameterization
	float     paramU;
	float     paramV;

	Material* material;

public:
	void BuildOrthonormalBasis();
	vec3 LocalToWorld(const vec3& localDirection) const;
	vec3 WorldToLocal(const vec3& worldDirection) const;
private:
	vec3 tangent;
	vec3 bitangent;
};

class Hitable
{

public:
	virtual ~Hitable() = default;

	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const = 0;

	// Returns false if bounding box is not supported
	virtual bool BoundingBox(float t0, float t1, AABB& outBox) const = 0;

};

class HitableList : public Hitable
{

public:
	HitableList() {}
	HitableList(std::vector<Hitable*> inList)
		: list(inList)
	{}

	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const;

	virtual bool BoundingBox(float t0, float t1, AABB& outBox) const override
	{
		if (list.size() == 0) return false;

		AABB box;
		bool first_true = list[0]->BoundingBox(t0, t1, box);
		if (!first_true)
		{
			return false;
		}
		for (auto i = 1; i < list.size(); ++i)
		{
			AABB tempBox;
			if (list[i]->BoundingBox(t0, t1, tempBox))
			{
				box = box + tempBox;
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	std::vector<Hitable*> list;
};
