#include "ShotgunBehavior.h"

void ShotgunBehavior::Shot(const ShotContext& c) {
	for (int i = -1; i <= 1; ++i) {
		FireBullet(c, MathUtility::DegreeToRadian(i * 15.0f));
	}
}
