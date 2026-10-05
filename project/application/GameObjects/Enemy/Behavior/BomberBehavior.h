#pragma once
#include "EnemyBehaviorBase.h"

/// <summary>
/// 爆弾タイプの敵挙動
/// </summary>
class BomberBehavior : public EnemyBehaviorBase {
public:
	EnemyType GetType() const override { return EnemyType::Bomber; }
	int GetMaxHP() const override { return 1; }
	Vector4 GetColor() const override { return {0, 0, 1, 1}; }

	float GetDeathBlastRadius() const override { return 5.0f; }

	void Shot(const ShotContext& c) override;

	void OnDeath(EngineContext* ctx, const Vector3& pos, std::list<std::unique_ptr<TinyEngine::Particle>>& container) override;
};