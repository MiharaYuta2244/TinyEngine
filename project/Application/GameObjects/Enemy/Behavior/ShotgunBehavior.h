#pragma once
#include "EnemyBehaviorBase.h"

/// <summary>
/// ショットガンタイプの敵挙動
/// </summary>
class ShotgunBehavior : public EnemyBehaviorBase {
public:
	EnemyType GetType() const override { return EnemyType::Shotgun; }
	int GetMaxHP() const override { return 2; }
	void Shot(const ShotContext& c) override;
};
