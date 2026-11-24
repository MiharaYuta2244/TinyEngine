#include "AssaultBehavior.h"
#include "Random.h"

void AssaultBehavior::Shot(const ShotContext& c) {
	FireBullet(c, MathUtility::DegreeToRadian(RandomUtils::RangeFloat(-20.0f, 20.0f)));
}
