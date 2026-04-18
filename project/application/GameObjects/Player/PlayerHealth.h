#pragma once
#include "GameTimer.h"

/// <summary>
/// プレイヤーのHP管理クラス
/// </summary>
class PlayerHealth {
public:
	// 初期化処理
	void Initialize(float maxHP);

	// 更新処理
	void Update(float deltaTime);

	// ダメージ処理
	void Damage(float value);

	// 回復処理
	void AllHeal();

	// ImGui
	void DrawImGui();

	// 死亡フラグGetter
	bool IsDead() const;

private:
	float maxHP_ = 3.0f; // 最大HP
	float hp_ = maxHP_; // 現在のHP

	// 無敵状態フラグ
	bool isInvincible_ = false;

	// 無敵時間
	GameTimer invincibleTimer_;

	// 無敵時間
	float invincibleDuration_ = 1.0f;
};
