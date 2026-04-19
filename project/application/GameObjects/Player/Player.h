#pragma once
#include "AABB.h"
#include "DirectInput.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "PlayerHealth.h"
#include "PlayerMove.h"

class Enemy;

/// <summary>
/// プレイヤーの処理をまとめたクラス
/// </summary>
class Player {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, DirectInput* input, Enemy* enemy);

	// 描画処理
	void Draw();

	// 座標のGetter
	Vector3 GetPosition() const { return transform_.translate; }

	// 死亡フラグGetter
	bool IsDead() const;

	// ダメージ処理
	void Damage(float value);

	// 攻撃用の当たり判定Getter
	AABB GetAttackCol() const { return attackCol_; }

	// 攻撃可能かどうかSetter
	void SetEnableAttack(bool enableAttack) { enableAttack_ = enableAttack; }

private:
	Transform transform_;
	Vector2 velocity_;
	AABB attackCol_;
	Vector2 lastMoveDirection_;

	// プレイヤーの最大HP
	float maxHP_ = 100.0f;

	// 攻撃可能かどうかを表す変数
	bool enableAttack_ = false;

	bool isHold_ = false;

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス
	std::unique_ptr<PlayerMove> move_;     // 移動用インスタンス
	std::unique_ptr<PlayerHealth> hp_;     // HP管理用インスタンス
};
