#include "hit.h"

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
