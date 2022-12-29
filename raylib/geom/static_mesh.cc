#include "static_mesh.h"
#include "geom/bvh.h"

#define USE_BVH 1

StaticMesh::~StaticMesh()
{
	if (bvh != nullptr)
	{
		delete bvh;
	}
}

void StaticMesh::AddTriangle(const Triangle& triangle)
{
	CHECK(!bLocked);
	if (!bLocked)
	{
		triangles.push_back(triangle);
	}
}

void StaticMesh::SetBounds(const AABB& inBounds)
{
	CHECK(!bLocked);
	if (!bLocked)
	{
		bounds = inBounds;
		boundsValid = true;
	}
}

void StaticMesh::CalculateBounds()
{
	CHECK(!bLocked);
	if (!bLocked)
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
	CHECK(!bLocked);
	if (!bLocked)
	{
		Transform rot = transform;
		rot.SetLocation(vec3(0.0f, 0.0f, 0.0f));
		rot.SetScale(vec3(1.0f, 1.0f, 1.0f));

		for (Triangle& T : triangles)
		{
			std::vector<vec3> vs(3);
			std::vector<vec3> ns(3);
			T.GetVertices(vs[0], vs[1], vs[2]);
			T.GetNormals(ns[0], ns[1], ns[2]);

			transform.TransformVectors(vs);
			rot.TransformVectors(ns);

			T.SetVertices(vs[0], vs[1], vs[2]);
			T.SetNormals(ns[0], ns[1], ns[2]);
		}
		boundsValid = false;
	}
}

void StaticMesh::Finalize()
{
	if (!bLocked)
	{
		CalculateBounds();

		std::vector<Hitable*> triVec(triangles.size());
		for (auto i = 0; i < triangles.size(); ++i)
		{
			triVec[i] = &triangles[i];
		}
		bvh = new BVHNode(new HitableList(triVec), 0.0f, 0.0f);

		bLocked = true;
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
