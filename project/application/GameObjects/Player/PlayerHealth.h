#pragma once
class PlayerHealth {
public:
	// 初期化処理
	void Initialize(float maxHP);

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
};
