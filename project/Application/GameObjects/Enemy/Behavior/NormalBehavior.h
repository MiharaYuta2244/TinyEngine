#pragma once
#include "EnemyBehaviorBase.h"

/// <summary>
/// 通常の敵挙動
/// </summary>
class NormalBehavior : public EnemyBehaviorBase {
public:
	EnemyType GetType() const override { return EnemyType::Normal; }
	int GetMaxHP() const override { return 1; }

	void Shot(const ShotContext& c) override;
};
