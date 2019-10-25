#include "static_mesh.h"

void StaticMesh::AddTriangle(const Triangle& triangle)
{
	triangles.push_back(triangle);
}

void StaticMesh::ApplyTransform(const Transform& transform)
{
	for (Triangle& T : triangles)
	{
		std::vector<vec3> vs(3, vec3(0.0f,0.0f,0.0f));
		T.GetVertices(vs[0], vs[1], vs[2]);
		transform.TransformVectors(vs);
		T.SetVertices(vs[0], vs[1], vs[2]);
	}
}

bool StaticMesh::Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const
{
	HitResult temp;
	bool anyHit = false;
	float closest = t_max;
	int32 n = (int32)triangles.size();
	int32 ix;
	for (int32 i = 0; i < n; ++i)
	{
		if (triangles[i].Hit(r, t_min, closest, temp))
		{
			anyHit = true;
			closest = temp.t;
			outResult = temp;
			ix = i;
		}
	}

	return anyHit;
}
