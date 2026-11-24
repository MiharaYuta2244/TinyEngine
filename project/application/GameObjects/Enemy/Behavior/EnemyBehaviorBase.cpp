#include "EnemyBehaviorBase.h"
#include "GameObjects/Enemy/Weapon/EnemyBulletManager.h"
#include <cmath>

void EnemyBehaviorBase::FireBullet(const ShotContext& c, float yawOffset) {
	const float sinYaw = std::sin(yawOffset);
	const float cosYaw = std::cos(yawOffset);
	const Vector3& aim = c.beamDirection;
	const Vector3 direction{aim.x * cosYaw + aim.z * sinYaw, aim.y, aim.z * cosYaw - aim.x * sinYaw};
	auto bullet = std::make_unique<EnemyBullet>();
	bullet->Initialize(c.engine, c.muzzle, direction, c.shooterId);
	c.bullets->AddBullet(std::move(bullet));
}
