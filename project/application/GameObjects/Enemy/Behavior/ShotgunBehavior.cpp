#include "ShotgunBehavior.h"

void ShotgunBehavior::Shot(const ShotContext& c) {
	Vector3 d = MathUtility::Normalize(c.toTarget);
	float base = std::atan2(d.x, d.z);
	for (int i = -1; i <= 1; ++i) {
		float a = base + i * MathUtility::DegreeToRadian(15.0f);
		FireBullet(c, {std::sin(a), std::cos(a)});
	}
}