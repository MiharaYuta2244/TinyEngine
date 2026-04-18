#include "EnemyAI.h"
#include "MathUtility.h"
#include <algorithm>
#include <cmath>
#include <numbers>

void EnemyAI::Initialize(Transform* transform, EngineContext* ctx) {
	transform_ = transform;
	ctx_ = ctx;

	// 弾の発射間隔を設定
	shotTimer_.Initialize(shotDuration_);
}

void EnemyAI::Update(float deltaTime, const Vector3& playerPos) {
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
			// 弾を生成
			BulletGenerate();

			// タイマーのリセット
			shotTimer_.Initialize(shotDuration_);
		}

		// 弾の更新処理
		for (auto& bullet : bullets_) {
			bullet->Update(deltaTime, bulletSpeed_);

			// 弾の削除
			std::erase_if(bullets_, [this](const std::unique_ptr<EnemyBullet>& b) {
				return b->IsDead(bulletActiveArea_); // 消したい条件
			});
		}
		break;
	}

#ifdef USE_IMGUI
	ImGui::Begin("bullet");
	ImGui::DragFloat("speed", &bulletSpeed_, 0.01f);
	ImGui::End();
#endif
}

void EnemyAI::Draw() {
	// 弾の描画
	for (auto& bullet : bullets_) {
		bullet->Draw();
	}
}

void EnemyAI::BulletGenerate() {
	// 弾の生成&初期化
	auto bullet = std::make_unique<EnemyBullet>();
	Vector3 dir = MathUtility::Normalize(playerPos_);
	bullet->Initialize(ctx_, {dir.x, dir.z});
	bullets_.push_back(std::move(bullet));
}
