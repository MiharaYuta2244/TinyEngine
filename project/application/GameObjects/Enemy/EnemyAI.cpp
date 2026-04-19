#include "EnemyAI.h"
#include "MathUtility.h"
#include "EnemyBulletManager.h"
#include "MathOperator.h"
#include <algorithm>
#include <cmath>
#include <numbers>

void EnemyAI::Initialize(Transform* transform, EngineContext* ctx) {
	transform_ = transform;
	ctx_ = ctx;

	// 弾の発射間隔を設定
	shotTimer_.Initialize(shotDuration_);
}

void EnemyAI::Update(float deltaTime, const Vector3& playerPos, EnemyBulletManager* enemyBulletManager) {
	// プレイヤーの座標を受け取る
	playerPos_ = playerPos;

	switch (state_) {
	case State::Normal:

		break;

	case State::Vigilance:

		break;

	case State::Shot:
		// 弾を発射するまでの時間を計測
		shotTimer_.Update(deltaTime);

		// 指定した時間がたったら
		if (shotTimer_.IsEnd()) {
			// 弾の生成&初期化
			auto bullet = std::make_unique<EnemyBullet>();
			Vector3 enemyPos = transform_->translate;
			Vector3 dir = MathUtility::Normalize(playerPos_ - enemyPos);
			bullet->Initialize(ctx_, {dir.x, dir.z}, transform_->translate);
			enemyBulletManager->AddBullet(std::move(bullet));

			// タイマーのリセット
			shotTimer_.Initialize(shotDuration_);
		}
		break;
	}
}
