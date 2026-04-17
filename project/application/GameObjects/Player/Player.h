#pragma once
#include "PlayerRender.h"
#include "PlayerMove.h"
#include "DirectInput.h"

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

	// ダメージ
	void Damage(int value);

	// 座標のGetter
	Vector3 GetPosition() const { return transform_.translate; }

	// 死亡フラグGetter
	bool IsDead() const;

private:
	Transform transform_;
	Vector2 velocity_;

	std::unique_ptr<PlayerRender> render_; // 描画用インスタンス
	std::unique_ptr<PlayerMove> move_; // 移動用インスタンス
};
