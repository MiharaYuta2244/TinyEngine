#pragma once
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "PlayerMove.h"
#include "DirectInput.h"
#include "PlayerHealth.h"

/// <summary>
/// プレイヤーの処理をまとめたクラス
/// </summary>
class Player {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, DirectInput* input);

	// 描画処理
	void Draw();

	// 座標のGetter
	Vector3 GetPosition() const { return transform_.translate; }

	// 死亡フラグGetter
	bool IsDead() const;

	// ダメージ処理
	void Damage(float value);

private:
	Transform transform_;
	Vector2 velocity_;

	// プレイヤーの最大HP
	float maxHP_ = 3.0f;

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス
	std::unique_ptr<PlayerMove> move_; // 移動用インスタンス
	std::unique_ptr<PlayerHealth> hp_; // HP管理用インスタンス
};
