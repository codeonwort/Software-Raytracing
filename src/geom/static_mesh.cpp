#include "static_mesh.h"

void StaticMesh::AddTriangle(const Triangle& triangle)
{
	triangles.push_back(triangle);
}

bool StaticMesh::Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const
{
	HitResult temp;
	bool anyHit = false;
	float closest = t_max;
	int32 n = (int32)triangles.size();
	for (int32 i = 0; i < n; ++i)
	{
		if (triangles[i].Hit(r, t_min, closest, temp))
		{
			anyHit = true;
			closest = temp.t;
			outResult = temp;
		}
	}
	return anyHit;
}
