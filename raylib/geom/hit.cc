#include "hit.h"

// -------------------------------
// HitResult

inline void CalcOrthonormalBasis(const vec3& N, vec3& T, vec3& B) {
	if (std::abs(N.x) > 0.9f) {
		T = vec3(0.0f, 1.0f, 0.0f);
	} else {
		T = vec3(1.0f, 0.0f, 0.0f);
	}
	B = normalize(cross(T, N));
	T = normalize(cross(N, B));
}

void HitResult::BuildOrthonormalBasis() {
	CalcOrthonormalBasis(n, tangent, bitangent);
}

vec3 HitResult::LocalToWorld(const vec3& v) const {
	float wx = dot(vec3(tangent.x, bitangent.x, n.x), v);
	float wy = dot(vec3(tangent.y, bitangent.y, n.y), v);
	float wz = dot(vec3(tangent.z, bitangent.z, n.z), v);
	return vec3(wx, wy, wz);
}

vec3 HitResult::WorldToLocal(const vec3& v) const {
	return vec3(dot(v, tangent), dot(v, bitangent), dot(v, n));
}

// -------------------------------
// HitableList

bool HitableList::Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const
{
	HitResult temp;
	bool anyHit = false;
	float closest = t_max;
	int32 n = (int32)hitables.size();
	for (int32 i = 0; i < n; ++i)
	{
		if (hitables[i]->Hit(r, t_min, closest, temp))
		{
			anyHit = true;
			closest = temp.t;
			outResult = temp;
		}
	}
	return anyHit;
}
