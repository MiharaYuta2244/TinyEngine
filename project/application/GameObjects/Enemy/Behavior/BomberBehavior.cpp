#include "BomberBehavior.h"
#include "GameObjects/Effect/EffectGenerator.h"
#include "GameObjects/Enemy/Weapon/EnemyBombManager.h"

void BomberBehavior::Shot(const ShotContext& c) {
	float distSq = c.toTarget.x * c.toTarget.x + c.toTarget.z * c.toTarget.z;
	Vector3 d = MathUtility::Normalize(c.toTarget);
	if (!c.bombs || distSq <= 15.0f * 15.0f) {
		FireBullet(c, {d.x, d.z});
	} else {
		auto bomb = std::make_unique<EnemyBomb>();
		bomb->Initialize(c.engine, c.origin, c.lastKnownPlayerPos);
		c.bombs->AddBomb(std::move(bomb));
	}
}

void BomberBehavior::OnDeath(EngineContext* ctx, const Vector3& pos, std::list<std::unique_ptr<TinyEngine::Particle>>& container) {
	EffectGenerator::CreateBomberExplosionEffect(ctx, pos, GetDeathBlastRadius(), container);
}