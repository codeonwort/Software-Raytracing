#include "static_mesh.h"
#include "bvh.h"

// #todo-staticmesh: Still too slow for bedroom
#define USE_BVH 1

void StaticMesh::AddTriangle(const Triangle& triangle)
{
	CHECK(!locked);
	if (!locked)
	{
		triangles.push_back(triangle);
	}
}

void StaticMesh::SetBounds(const AABB& inBounds)
{
	CHECK(!locked);
	if (!locked)
	{
		bounds = inBounds;
		boundsValid = true;
	}
}

void StaticMesh::CalculateBounds()
{
	CHECK(!locked);
	if (!locked)
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
}

void StaticMesh::ApplyTransform(const Transform& transform)
{
	CHECK(!locked);
	if (!locked)
	{
		for (Triangle& T : triangles)
		{
			std::vector<vec3> vs(3, vec3(0.0f, 0.0f, 0.0f));
			T.GetVertices(vs[0], vs[1], vs[2]);
			transform.TransformVectors(vs);
			T.SetVertices(vs[0], vs[1], vs[2]);
		}
		boundsValid = false;
	}
}

void StaticMesh::Finalize()
{
	if (!locked)
	{
		CalculateBounds();

		std::vector<Hitable*> triVec(triangles.size());
		for (auto i = 0; i < triangles.size(); ++i)
		{
			triVec[i] = &triangles[i];
		}
		bvh = new BVHNode(new HitableList(triVec), 0.0f, 0.0f);

		locked = true;
	}
}

bool StaticMesh::Hit(const ray& r, float t_min, float t_max, HitResult& outResult) const
{
	if (!boundsValid)
	{
		CHECK_NO_ENTRY();
	}
	else if (!bounds.Hit(r, t_min, t_max))
	{
		return false;
	}

#if USE_BVH
	return bvh->Hit(r, t_min, t_max, outResult);
#else
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
#endif
}

bool StaticMesh::BoundingBox(float t0, float t1, AABB& outBox) const
{
	outBox = bounds;
	return boundsValid;
}
