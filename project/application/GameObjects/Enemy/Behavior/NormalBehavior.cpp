#include "NormalBehavior.h"

void NormalBehavior::Shot(const ShotContext& c) {
	Vector3 d = MathUtility::Normalize(c.toTarget);
	FireBullet(c, {d.x, d.z});
}