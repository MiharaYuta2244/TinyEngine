#include "BomberBehavior.h"
#include "Effect/EffectGenerator.h"
#include "GameObjects/Enemy/Weapon/EnemyBomb.h"
#include "GameObjects/Enemy/Weapon/EnemyWeaponManager.h"
#include <utility>

void BomberBehavior::Shot(const ShotContext& c) {
	float distSq = c.toTarget.x * c.toTarget.x + c.toTarget.z * c.toTarget.z;
	if (distSq <= 15.0f * 15.0f) {
		FireBullet(c);
	} else {
		auto bomb = std::make_unique<EnemyBomb>();
		bomb->Initialize(c.engine, c.origin, c.lastKnownPlayerPos);
		c.weapons.AddBomb(std::move(bomb));
	}
}

void BomberBehavior::OnDeath(EngineContext* ctx, const Vector3& pos, std::list<std::unique_ptr<TinyEngine::Particle>>& container) {
	EffectGenerator::CreateBomberExplosionEffect(ctx, pos, GetDeathBlastRadius(), container);
}
