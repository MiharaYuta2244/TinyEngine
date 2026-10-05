#include "BomberBehavior.h"
#include "GameObjects/Effect/EffectGenerator.h"
#include "GameObjects/Enemy/Weapon/EnemyBombManager.h"

void BomberBehavior::Shot(const ShotContext& c) {
	float distSq = c.toTarget.x * c.toTarget.x + c.toTarget.z * c.toTarget.z;
	Vector3 d = MathUtility::Normalize(c.toTarget);
	if (distSq <= 15.0f * 15.0f && c.bombs) {
		FireBullet(c, {d.x, d.z});
	} else {
		auto bomb = std::make_unique<EnemyBomb>();
		bomb->Initialize(c.engine, c.origin, {d.x * 18.0f, 0, d.z * 18.0f}, c.lastKnownPlayerPos);
		c.bombs->AddBomb(std::move(bomb));
	}
}

void BomberBehavior::OnDeath(EngineContext* ctx, const Vector3& pos, std::vector<std::unique_ptr<TinyEngine::Particle>>& deathEffect) { EffectGenerator::CreateHitEffect(ctx, pos, deathEffect); }