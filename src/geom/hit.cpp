#include "hit.h"

// -------------------------------
// HitResult

inline void CalcOrthonormalBasis(const vec3& N, vec3& T, vec3& B) {
	if (std::abs(N.x) > 0.9f) {
		T = vec3(0.0f, 1.0f, 0.0f);
	} else {
		T = vec3(1.0f, 0.0f, 0.0f);
	}
	B = cross(T, N);
	T = cross(N, B);
}

void HitResult::BuildOrthonormalBasis() {
	CalcOrthonormalBasis(n, tangent, bitangent);
}

vec3 HitResult::LocalToWorld(const vec3& v) {
	float wx = tangent.x * v.x + bitangent.x * v.y + n.x * v.z;
	float wy = tangent.y * v.x + bitangent.x * v.y + n.y * v.z;
	float wz = tangent.z * v.x + bitangent.z * v.y + n.z * v.z;
	return vec3(wx, wy, wz);
}

vec3 HitResult::WorldToLocal(const vec3& v) {
	return vec3(dot(v, tangent), dot(v, bitangent), dot(v, n));
}

// -------------------------------
// HitableList

bool HitableList::Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const
{
	HitResult temp;
	bool anyHit = false;
	float closest = t_max;
	int32 n = (int32)list.size();
	for (int32 i = 0; i < n; ++i)
	{
		if (list[i]->Hit(r, t_min, closest, temp))
		{
			anyHit = true;
			closest = temp.t;
			outResult = temp;
		}
	}
	return anyHit;
}
