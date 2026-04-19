#pragma once
#include "AABB.h"
#include "EnemyAI.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "AnimationBundle.h"

class EnemyBulletManager;

class Enemy {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, Vector3 playerPos, EnemyBulletManager* enemyBulletManager);

	// 描画処理
	void Draw();

	// 死亡フラグGetter
	bool IsDead() const;

	// 敵の当たり判定
	AABB GetBodyCol() const { return bodyCol_; }

	// 敵の座標Getter
	Vector3& GetPos() { return transform_.translate; }

	// 敵の座標Setter
	void SetPos(Vector3 pos) { transform_.translate = pos; }

	void SetEnableMove(bool enableMove) { enableMove_ = enableMove; }

	// ノックバックを始める
	void StartKnockBack(Vector3 dir);

private:
	Transform transform_;
	AABB bodyCol_; // 本体のAABB
	bool enableMove_ = true;
	AnimationBundle<Vector3> knockBackAnim_;

	// ノックバックの強さ
	float knockBackPower_ = 10.0f;

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス
	std::unique_ptr<EnemyAI> ai_;          // AI
};
