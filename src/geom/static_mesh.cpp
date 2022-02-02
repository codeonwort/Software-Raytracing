#include "static_mesh.h"

void StaticMesh::AddTriangle(const Triangle& triangle)
{
	triangles.push_back(triangle);
}

void StaticMesh::SetBounds(const AABB& inBounds)
{
	bounds = inBounds;
	boundsValid = true;
}

void StaticMesh::CalculateBounds()
{
	vec3 minBounds(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
	vec3 maxBounds(-FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX);

	for (const Triangle& T : triangles)
	{
		vec3 v0, v1, v2;
		T.GetVertices(v0, v1, v2);
		minBounds = min(min(min(minBounds, v0), v1), v2);
		maxBounds = max(max(max(maxBounds, v0), v1), v2);
	}

	bounds = AABB(minBounds, maxBounds);
	boundsValid = true;
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
	boundsValid = false;
}

bool StaticMesh::Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const
{
	if (!boundsValid)
	{
		// #todo-staticmesh: Warning for performance?
	}
	else
	{
		if (!bounds.Hit(r, t_min, t_max))
		{
			return false;
		}
	}

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

bool StaticMesh::BoundingBox(float t0, float t1, AABB& outBox) const
{
	outBox = bounds;
	return boundsValid;
}
