#pragma once
#include "EnemyBullet.h"
#include "GameTimer.h"
#include "Rect.h"
#include "Transform.h"
#include <vector>

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
	void Update(float deltaTime, const Vector3& playerPos);

	// 描画処理
	void Draw();

private:
	// 弾の生成
	void BulletGenerate();

private:
	Transform* transform_ = nullptr;
	EngineContext* ctx_;

	// プレイヤーの座標
	Vector3 playerPos_;

	// 敵の状態
	State state_ = State::Shot;

	// 弾
	std::vector<std::unique_ptr<EnemyBullet>> bullets_;

	// 弾の発射タイマー
	GameTimer shotTimer_;

	// 弾の発射間隔
	float shotDuration_ = 2.0f;

	// 弾が生存できる範囲
	Rect<float> bulletActiveArea_ = {10.0f, -10.0f, 10.0f, -10.0f};

	// 弾の速度
	float bulletSpeed_ = 20.0f;
};
