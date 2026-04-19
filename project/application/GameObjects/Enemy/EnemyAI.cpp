#include "EnemyAI.h"
#include "MathUtility.h"
#include "EnemyBulletManager.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/Wall/WallManager.h"
#include "MathOperator.h"
#include <algorithm>
#include <cmath>

void EnemyAI::Initialize(Transform* transform, EngineContext* ctx) {
	transform_ = transform;
	ctx_ = ctx;

	// 弾の発射間隔を設定
	shotTimer_.Initialize(shotDuration_);

	// 警戒時間を設定
	vigilanceTimer_.Initialize(vigilanceDuration_);
}

void EnemyAI::Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager) {
	Vector3 playerPos = player->GetPosition();
	Vector3 enemyPos = transform_->translate;

	// プレイヤーとの平面距離を計算
	Vector3 toPlayer = {playerPos.x - enemyPos.x, 0.0f, playerPos.z - enemyPos.z};
	float distToPlayer = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);

	// 射線の確保状況
	bool hasLineOfSight = CheckLineOfSight(enemyPos, playerPos, wallManager);

	switch (state_) {
	case State::Normal: {
		// 1. 索敵処理 (前方AABB内にプレイヤーが入ったか)
		AABB sArea = GetSearchArea();
		AABB pCol = player->GetBodyCol();
		bool isHit = (sArea.max.x >= pCol.min.x && sArea.min.x <= pCol.max.x && sArea.max.y >= pCol.min.y && sArea.min.y <= pCol.max.y && sArea.max.z >= pCol.min.z && sArea.min.z <= pCol.max.z);

		if (isHit) {
			state_ = State::Vigilance;
			vigilanceTimer_.Initialize(vigilanceDuration_); // 警戒タイマーセット
		}
		break;
	}

	case State::Vigilance: {
		vigilanceTimer_.Update(deltaTime);

		// 2. 射程圏内で、かつ壁に遮られていなければ射撃モードへ
		if (distToPlayer <= attackRange_ && hasLineOfSight) {
			state_ = State::Shot;
			shotTimer_.Initialize(shotDuration_); // 即撃ちを防ぐためタイマーリセット
		} else {
			// 3. プレイヤーを追いかける ＋ 壁の回避処理
			if (distToPlayer > 0.01f) {
				Vector3 moveDir = {toPlayer.x / distToPlayer, 0.0f, toPlayer.z / distToPlayer};

				// 【簡易壁回避】壁が近くにある場合、壁から遠ざかる反発力（斥力）を移動ベクトルに足す
				for (const auto& wall : wallManager->GetWalls()) {
					AABB wCol = wall->GetCollision();
					Vector3 wCenter = {(wCol.max.x + wCol.min.x) * 0.5f, 0.0f, (wCol.max.z + wCol.min.z) * 0.5f};
					Vector3 toWall = {wCenter.x - enemyPos.x, 0.0f, wCenter.z - enemyPos.z};
					float distToWall = std::sqrt(toWall.x * toWall.x + toWall.z * toWall.z);

					float avoidRange = 3.0f; // 回避を始める距離
					if (distToWall > 0.01f && distToWall < avoidRange) {
						// 壁から離れる方向へのベクトル
						float power = (avoidRange - distToWall) / avoidRange;
						moveDir.x -= (toWall.x / distToWall) * power * 1.5f;
						moveDir.z -= (toWall.z / distToWall) * power * 1.5f;
					}
				}

				// 合成されたベクトルを正規化して移動に適用
				if (moveDir.x != 0.0f || moveDir.z != 0.0f) {
					Vector3 moveDir3 = {moveDir.x, 0.0f, moveDir.z};
					moveDir = MathUtility::Normalize(moveDir3);
					transform_->translate.x += moveDir.x * moveSpeed_ * deltaTime;
					transform_->translate.z += moveDir.z * moveSpeed_ * deltaTime;
				}
			}
		}

		// 4. 一定時間経過で通常状態に戻る
		if (vigilanceTimer_.IsEnd()) {
			state_ = State::Normal;
		}
		break;
	}

	case State::Shot: {
		// 5. 壁に隠れられたり、射程外に逃げられたら警戒モードに戻る
		if (!hasLineOfSight || distToPlayer > attackRange_) {
			state_ = State::Vigilance;
			vigilanceTimer_.Initialize(vigilanceDuration_);
			break;
		}

		// 既存の射撃処理
		shotTimer_.Update(deltaTime);
		if (shotTimer_.IsEnd()) {
			auto bullet = std::make_unique<EnemyBullet>();
			Vector3 dir = MathUtility::Normalize(toPlayer);
			bullet->Initialize(ctx_, {dir.x, dir.z}, transform_->translate);
			enemyBulletManager->AddBullet(std::move(bullet));

			shotTimer_.Initialize(shotDuration_);
		}
		break;
	}
	}
}

AABB EnemyAI::GetSearchArea() const {
	AABB area;
	Vector3 pos = transform_->translate;
	area.min = {pos.x - searchWidth_ * 0.5f, pos.y - 1.0f, pos.z};
	area.max = {pos.x + searchWidth_ * 0.5f, pos.y + 1.0f, pos.z + searchDistance_};
	return area;
}

bool EnemyAI::CheckLineOfSight(const Vector3& start, const Vector3& end, WallManager* wallManager) {
	Vector3 dir = {end.x - start.x, end.y - start.y, end.z - start.z};

	for (const auto& wall : wallManager->GetWalls()) {
		AABB aabb = wall->GetCollision();
		float tmin = 0.0f;
		float tmax = 1.0f;

		auto checkAxis = [&](float pStart, float pDir, float bMin, float bMax) -> bool {
			if (std::abs(pDir) < 0.0001f) {
				return (pStart >= bMin && pStart <= bMax);
			}
			float t1 = (bMin - pStart) / pDir;
			float t2 = (bMax - pStart) / pDir;
			if (t1 > t2)
				std::swap(t1, t2);
			if (tmin < t1)
				tmin = t1;
			if (tmax > t2)
				tmax = t2;
			return tmin <= tmax;
		};

		if (!checkAxis(start.x, dir.x, aabb.min.x, aabb.max.x))
			continue;
		if (!checkAxis(start.y, dir.y, aabb.min.y, aabb.max.y))
			continue;
		if (!checkAxis(start.z, dir.z, aabb.min.z, aabb.max.z))
			continue;

		// 1つでも交差していれば射線が遮られている
		return false;
	}
	return true; // 全ての壁をすり抜けた＝射線が通っている 
}
