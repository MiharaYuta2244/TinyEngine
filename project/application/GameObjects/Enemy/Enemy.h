#pragma once
#include "AABB.h"
#include "AnimationBundle.h"
#include "EnemyAI.h"
#include "GameObjects/ObjectRender/ObjectRender.h"

class EnemyBulletManager;

class Enemy {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, Vector3 playerPos, EnemyBulletManager* enemyBulletManager);

	// 押し戻し完了後に呼ぶ最終更新処理
	void PostUpdate();

	// 描画処理
	void Draw();

	// 死亡フラグGetter
	bool IsDead() const { return isDead_; }

	// 敵の当たり判定
	AABB GetBodyCol() const { return bodyCol_; }

	// 敵の座標Getter
	Vector3& GetPos() { return transform_.translate; }

	// 敵の座標Setter
	void SetPos(Vector3 pos) { transform_.translate = pos; }

	void SetEnableMove(bool enableMove) { enableMove_ = enableMove; }

	// ノックバックを始める
	void StartKnockBack(Vector3 dir);

	// ノックバック中かどうかを取得
	bool IsKnockBack() const { return knockBackAnim_.anim.GetIsActive(); }

	// 死亡させる処理
	void Kill() { isDead_ = true; }

private:
	// 当たり判定の更新
	void UpdateCollision();

private:
	Transform transform_;
	AABB bodyCol_; // 本体のAABB
	bool enableMove_ = true;
	AnimationBundle<Vector3> knockBackAnim_;
	bool isDead_ = false;

	// ノックバックの強さ
	float knockBackPower_ = 10.0f;

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス
	std::unique_ptr<EnemyAI> ai_;          // AI
};
