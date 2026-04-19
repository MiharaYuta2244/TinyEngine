#pragma once
#include "AABB.h"
#include "DirectInput.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "PlayerHealth.h"
#include "PlayerMove.h"

class EnemyManager;

/// <summary>
/// プレイヤーの処理をまとめたクラス
/// </summary>
class Player {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, DirectInput* input, EnemyManager* enemyManager);

	// 当たり判定などの解決後に行う最終更新処理
	void PostUpdate();

	// 描画処理
	void Draw();

	// 座標のGetter
	Vector3 GetPosition() const { return transform_.translate; }

	// 座標のSetter
	void SetPosition(const Vector3& pos) { transform_.translate = pos; }

	// 死亡フラグGetter
	bool IsDead() const;

	// ダメージ処理
	void Damage(float value);

	// 攻撃用の当たり判定Getter
	AABB GetAttackCol() const { return attackCol_; }

	// 攻撃可能かどうかSetter
	void SetEnableAttack(bool enableAttack) { enableAttack_ = enableAttack; }

	// 本体の当たり判定
	AABB GetBodyCol() const { return bodyCol_; }

private:
	// 当たり判定の更新処理
	void UpdateCollision();

private:
	Transform transform_;
	Vector2 velocity_;
	AABB attackCol_;
	AABB bodyCol_;
	Vector2 lastMoveDirection_;

	// プレイヤーの最大HP
	float maxHP_ = 100.0f;

	// 攻撃可能かどうかを表す変数
	bool enableAttack_ = false;

	bool isHold_ = false;

	// つかんでいる敵のポインタを記憶するための変数
	class Enemy* heldEnemy_ = nullptr;

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス
	std::unique_ptr<PlayerMove> move_;     // 移動用インスタンス
	std::unique_ptr<PlayerHealth> hp_;     // HP管理用インスタンス
};
