#pragma once

#include "hit.h"
#include "material.h"

class sphere : public Hitable
{

public:
	sphere(const vec3& inCenter, float inRadius, Material* inMaterial)
		: center(inCenter)
		, radius(inRadius)
		, material(inMaterial)
	{
	}

	virtual bool Hit(const ray& r, float t_min, float t_max, HitResult& result) const;

	vec3 center;
	float radius;
	Material* material;

};

bool sphere::Hit(const ray& r, float t_min, float t_max, HitResult& result) const
{
	vec3 oc = r.o - center;
	float a = dot(r.d, r.d);
	float b = dot(oc, r.d);
	float c = dot(oc, oc) - radius * radius;
	float D = b*b - a*c;

	if(D > 0.0f)
	{
		float temp = (-b - sqrt(b*b - a*c)) / a;
		if(t_min < temp && temp < t_max)
		{
			result.t = temp;
			result.p = r.at(result.t);
			result.n = (result.p - center) / radius;
			result.material = material;
			return true;
		}
		temp = (-b + sqrt(b*b - a*c)) / a;
		if(t_min < temp && temp < t_max)
		{
			result.t = temp;
			result.p = r.at(result.t);
			result.n = (result.p - center) / radius;
			result.material = material;
			return true;
		}
	}
	return false;
}

