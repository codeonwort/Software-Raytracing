#include "sphere.h"

bool sphere::Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const
{
	vec3 oc = r.o - center;
	float a = dot(r.d, r.d);
	float b = dot(oc, r.d);
	float c = dot(oc, oc) - radius * radius;
	float D = b * b - a * c;

	if (D > 0.0f)
	{
		bool bHit = false;

		float temp = (-b - sqrt(b * b - a * c)) / a;
		if (t_min < temp && temp < t_max)
		{
			outResult.t = temp;
			outResult.p = r.at(outResult.t);
			outResult.n = (outResult.p - center) / radius;
			outResult.material = material;
			bHit = true;
		}
		if (!bHit)
		{
			temp = (-b + sqrt(b * b - a * c)) / a;
			if (t_min < temp && temp < t_max)
			{
				outResult.t = temp;
				outResult.p = r.at(outResult.t);
				outResult.n = (outResult.p - center) / radius;
				outResult.material = material;
				bHit = true;
			}
		}
		if (bHit)
		{
			vec3 op = outResult.p - center;
			outResult.paramU = ::atanf(op.y / op.x);
			outResult.paramV = ::acosf(op.z / radius);
		}
		return bHit;
	}
	return false;
}

bool sphere::BoundingBox(float t0, float t1, AABB& outBox) const
{
	vec3 R = vec3(radius, radius, radius);
	outBox = AABB(center - R, center + R);
	return true;
}
