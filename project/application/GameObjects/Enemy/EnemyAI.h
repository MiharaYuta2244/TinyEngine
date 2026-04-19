#pragma once
#include "EnemyBullet.h"
#include "GameTimer.h"
#include "Transform.h"
#include "AABB.h"
#include <vector>

class EnemyBulletManager;
class WallManager;
class Player;

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
	void Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager);

private:
	// 前方の索敵用AABBを生成する
	AABB GetSearchArea() const;

	// 敵とプレイヤーの間に壁がないか判定する
	bool CheckLineOfSight(const Vector3& start, const Vector3& end, WallManager* wallManager);

private:
	Transform* transform_ = nullptr;
	EngineContext* ctx_;

	// 敵の状態
	State state_ = State::Normal;

	// タイマー類
	GameTimer shotTimer_;// 弾の発射タイマー
	float shotDuration_ = 2.0f;// 弾の発射間隔

	GameTimer vigilanceTimer_;
	float vigilanceDuration_ = 5.0f; // 警戒モードの持続時間

	// AI用パラメータ
	float searchDistance_ = 10.0f; // 前方AABBの長さ
	float searchWidth_ = 4.0f;     // 前方AABBの幅
	float attackRange_ = 8.0f;     // 射撃モードに移行する距離
	float moveSpeed_ = 3.0f;       // 追跡時の移動速度
};
