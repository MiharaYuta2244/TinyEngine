#include "EnemyBehaviorBase.h"
#include "GameObjects/Enemy/Weapon/EnemyBulletManager.h"

void EnemyBehaviorBase::FireBullet(const ShotContext& c, Vector2 dir2D) {
	// 弾の発射
	auto bullet = std::make_unique<EnemyBullet>();
	Vector3 pos = c.origin;
	pos.x += dir2D.x * c.bulletMargin;
	pos.z += dir2D.y * c.bulletMargin;
	pos.y += 1.0f;
	bullet->Initialize(c.engine, dir2D, pos);
	c.bullets->AddBullet(std::move(bullet));
}