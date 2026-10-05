#pragma once
#include "EnemyBehaviorBase.h"

/// <summary>
/// ショットガンタイプの敵挙動
/// </summary>
class ShotgunBehavior : public EnemyBehaviorBase {
public:
	EnemyType GetType() const override { return EnemyType::Shotgun; }
	const char* GetName() const override { return "Shotgun"; }
	int GetMaxHP() const override { return 2; }
	Vector4 GetColor() const override { return {1, 0, 0, 1}; }
	void Shot(const ShotContext& c) override;
};