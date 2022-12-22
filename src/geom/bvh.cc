#include "bvh.h"
#include "core/random.h"
#include <algorithm>

BVHNode::BVHNode(HitableList* list, float t0, float t1)
	: BVHNode(list->hitables.data(), (int32)list->hitables.size(), t0, t1)
{
}

BVHNode::BVHNode(Hitable** list, int32 n, float t0, float t1)
{
	auto CompareX = [](const Hitable* L, const Hitable* R) -> bool
	{
		AABB left, right;
		if (!L->BoundingBox(0.0f, 0.0f, left) || !R->BoundingBox(0.0f, 0.0f, right))
		{
			// No bounding box in BVHNode ctor
			CHECK_NO_ENTRY();
		}
		return left.minBounds.x < right.minBounds.x;
	};
	auto CompareY = [](const Hitable* L, const Hitable* R) -> bool
	{
		AABB left, right;
		if (!L->BoundingBox(0.0f, 0.0f, left) || !R->BoundingBox(0.0f, 0.0f, right))
		{
			// No bounding box in BVHNode ctor
			CHECK_NO_ENTRY();
		}
		return left.minBounds.y < right.minBounds.y;
	};
	auto CompareZ = [](const Hitable* L, const Hitable* R) -> bool
	{
		AABB left, right;
		if (!L->BoundingBox(0.0f, 0.0f, left) || !R->BoundingBox(0.0f, 0.0f, right))
		{
			// No bounding box in BVHNode ctor
			CHECK_NO_ENTRY();
		}
		return left.minBounds.z < right.minBounds.z;
	};

	int32 axis = int32(Random() * 3);
	if (axis == 0)
	{
		std::sort(list, list + n, CompareX);
	}
	else if (axis == 1)
	{
		std::sort(list, list + n, CompareY);
	}
	else
	{
		std::sort(list, list + n, CompareZ);
	}

	if (n == 1)
	{
		// NOTE: Keep redundant references, skip right at Hit()
		left = right = list[0];
	}
	else if (n == 2)
	{
		left = list[0];
		right = list[1];
	}
	else
	{
		left = new BVHNode(list, n / 2, t0, t1);
		right = new BVHNode(list + n / 2, n - n / 2, t0, t1);
	}

	AABB leftBox, rightBox;
	if (!left->BoundingBox(t0, t1, leftBox) || !right->BoundingBox(t0, t1, rightBox))
	{
		// No bounding box in BVHNode ctor
		CHECK_NO_ENTRY();
	}
	box = leftBox + rightBox;
}

bool BVHNode::Hit(const ray& r, float tMin, float tMax, HitResult& outResult) const
{
	if (box.Hit(r, tMin, tMax))
	{
		HitResult leftResult, rightResult;
		bool leftHit = left->Hit(r, tMin, tMax, leftResult);
		// NOTE: Skip right if same node
		bool rightHit = (left == right) ? false : right->Hit(r, tMin, tMax, rightResult);
		if (leftHit && rightHit)
		{
			outResult = (leftResult.t < rightResult.t) ? leftResult : rightResult;
			return true;
		}
		else if (leftHit)
		{
			outResult = leftResult;
			return true;
		}
		else if (rightHit)
		{
			outResult = rightResult;
			return true;
		}
	}
	return false;
}

bool BVHNode::BoundingBox(float t0, float t1, AABB& outBox) const
{
	outBox = box;
	return true;
}
