#include "sphere.h"

bool sphere::Hit(const ray& r, float t_min, float t_max, HitResult& result) const
{
	vec3 oc = r.o - center;
	float a = dot(r.d, r.d);
	float b = dot(oc, r.d);
	float c = dot(oc, oc) - radius * radius;
	float D = b * b - a * c;

	if (D > 0.0f)
	{
		float temp = (-b - sqrt(b * b - a * c)) / a;
		if (t_min < temp && temp < t_max)
		{
			result.t = temp;
			result.p = r.at(result.t);
			result.n = (result.p - center) / radius;
			result.material = material;
			return true;
		}
		temp = (-b + sqrt(b * b - a * c)) / a;
		if (t_min < temp && temp < t_max)
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

bool sphere::BoundingBox(float t0, float t1, AABB& outBox) const
{
	vec3 R = vec3(radius, radius, radius);
	outBox = AABB(center - R, center + R);
	return true;
}
