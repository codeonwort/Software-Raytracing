#include "hit.h"

class BVHNode : public Hitable
{
	
public:
	BVHNode(HitableList* list, float t0, float t1);

	virtual bool Hit(const ray& r, float tMin, float tMax, HitResult& outResult) const override;

	virtual bool BoundingBox(float t0, float t1, AABB& outBox) const override;

	Hitable* left = nullptr;
	Hitable* right = nullptr;
	AABB box;

private:
	BVHNode(Hitable** list, int32 n, float t0, float t1);

};
