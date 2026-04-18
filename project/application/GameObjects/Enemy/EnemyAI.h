#pragma once
#include "EnemyBullet.h"
#include "GameTimer.h"
#include "Transform.h"
#include <vector>

class EnemyBulletManager;

/// <summary>
/// 敵AIクラス
/// </summary>
class EnemyAI {
public:
	enum class State {
		Normal,
		Vigilance,
		Shot,
	};

	// 初期化
	void Initialize(Transform* transform, EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, const Vector3& playerPos, EnemyBulletManager* enemyBulletManager);

private:
	Transform* transform_ = nullptr;
	EngineContext* ctx_;

	// プレイヤーの座標
	Vector3 playerPos_;

	// 敵の状態
	State state_ = State::Shot;

	// 弾の発射タイマー
	GameTimer shotTimer_;

	// 弾の発射間隔
	float shotDuration_ = 2.0f;
};
