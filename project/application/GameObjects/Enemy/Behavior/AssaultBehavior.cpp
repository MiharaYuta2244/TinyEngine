#include "AssaultBehavior.h"
#include "Random.h"

void AssaultBehavior::Shot(const ShotContext& c) {
	Vector3 d = MathUtility::Normalize(c.toTarget);
	float a = std::atan2(d.x, d.z) + MathUtility::DegreeToRadian(RandomUtils::RangeFloat(-20.0f, 20.0f));
	FireBullet(c, {std::sin(a), std::cos(a)});
}