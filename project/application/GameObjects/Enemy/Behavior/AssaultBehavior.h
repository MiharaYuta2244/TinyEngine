#pragma once
#include "EnemyBehaviorBase.h"

/// <summary>
/// アサルトタイプの敵挙動
/// </summary>
class AssaultBehavior : public EnemyBehaviorBase {
public:
	EnemyType GetType() const override { return EnemyType::Assault; }
	int GetMaxHP() const override { return 1; }
	Vector4 GetColor() const override { return {0, 1, 0, 1}; }
	float GetShotInterval() const override { return 0.4f; }
	void Shot(const ShotContext& c) override;
};