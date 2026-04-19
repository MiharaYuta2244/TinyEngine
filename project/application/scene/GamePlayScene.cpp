#include "GamePlayScene.h"
#include "Collision.h"
#include "Easing.h"
#include "Random.h"
#include "SceneManager.h"
#include <algorithm>
#include <fstream>

using namespace TinyEngine;

void GamePlayScene::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;
	sceneManager_ = sceneManager;

	// プレイヤーの生成&初期化
	player_ = std::make_unique<Player>();
	player_->Initialize(ctx);

	// 敵の生成&初期化
	enemy_ = std::make_unique<Enemy>();
	enemy_->Initialize(ctx);

	// 敵の弾管理インスタンス生成
	enemyBulletManager_ = std::make_unique<EnemyBulletManager>();

	// 壁の管理インスタンス生成&初期化
	wallManager_ = std::make_unique<WallManager>();
	wallManager_->Initialize(ctx);
}

void GamePlayScene::Update() {
	// プレイヤーの更新処理
	player_->Update(timeManager_->GetDeltaTime(), keyboard_, enemy_.get());

	// プレイヤーが死亡したらシーン遷移
	if (player_->IsDead()) {
		sceneManager_->ChangeScene("Title");
	}

	// 敵の更新処理
	enemy_->Update(timeManager_->GetDeltaTime(), player_->GetPosition(), enemyBulletManager_.get());

	// 敵の弾の更新処理
	enemyBulletManager_->Update(timeManager_->GetDeltaTime());

	// 壁の管理インスタンス更新
	wallManager_->Update();

	// 当たり判定
	CollisionGameObjects();

	// 押し戻し完了後の最終的な座標で、描画更新&AABB更新
	player_->PostUpdate();

	if (!enemy_->IsDead()) {
		enemy_->PostUpdate();
	}

	// 壁の管理インスタンスImGui
	wallManager_->DrawImGui();

#ifdef USE_IMGUI
	ImGui::Begin("Camera");
	ImGui::DragFloat3("Rotate", &debugCamera_->GetRotate().x, 0.01f);
	ImGui::DragFloat3("Translate", &debugCamera_->GetTranslation().x, 0.01f);
	ImGui::End();
#endif // USE_IMGUI
}

void GamePlayScene::Draw() {
	// プレイヤーの描画処理
	player_->Draw();

	// 敵の描画処理
	enemy_->Draw();

	// 敵の弾の描画処理
	enemyBulletManager_->Draw();

	// 壁の管理インスタンス描画
	wallManager_->Draw();
}

void GamePlayScene::Finalize() {}

void GamePlayScene::CollisionGameObjects() {
	// ==========================================
	// プレイヤーと敵の弾の当たり判定
	// ==========================================
	Sphere playerSphere = {player_->GetPosition(), 1.0f}; // プレイヤーの当たり判定形状（仮）
	for (const auto& bullet : enemyBulletManager_->GetBullets()) {
		Sphere bulletSphere = {bullet->GetPosition(), 0.5f}; // 弾の当たり判定形状（仮）

		// 衝突判定
		if (Collision::Intersect(playerSphere, bulletSphere)) {
			// プレイヤーにダメージを与える処理
			player_->Damage(1.0f);
		}
	}

	// ==========================================
	// プレイヤーの攻撃用範囲と敵の当たり判定
	// ==========================================
	if (Collision::Intersect(player_->GetAttackCol(), enemy_->GetBodyCol())) {
		// プレイヤーの攻撃フラグを立てる
		player_->SetEnableAttack(true);
	} else {
		// プレイヤーの攻撃フラグを下す
		player_->SetEnableAttack(false);
	}

	// ==========================================
	// プレイヤーと壁の押し出し判定
	// ==========================================
	AABB playerAABB = player_->GetBodyCol();
	Vector3 playerPos = player_->GetPosition();

	for (const auto& wall : wallManager_->GetWalls()) {
		AABB wallAABB = wall->GetCollision();

		// AABB同士の交差判定
		if (playerAABB.min.x <= wallAABB.max.x && playerAABB.max.x >= wallAABB.min.x && playerAABB.min.y <= wallAABB.max.y && playerAABB.max.y >= wallAABB.min.y &&
		    playerAABB.min.z <= wallAABB.max.z && playerAABB.max.z >= wallAABB.min.z) {

			// めり込み量の計算
			// X軸方向のめり込み量
			float overlapX1 = wallAABB.max.x - playerAABB.min.x; // 右から左へ押す量
			float overlapX2 = playerAABB.max.x - wallAABB.min.x; // 左から右へ押す量
			// Z軸方向のめり込み量
			float overlapZ1 = wallAABB.max.z - playerAABB.min.z; // 奥から手前へ押す量
			float overlapZ2 = playerAABB.max.z - wallAABB.min.z; // 手前から奥へ押す量

			// 最小のめり込み量を選ぶ（正の値にする）
			float minOverlapX = std::min(overlapX1, overlapX2);
			float minOverlapZ = std::min(overlapZ1, overlapZ2);

			// めり込みが少ない軸の方向に押し出す
			if (minOverlapX < minOverlapZ) {
				// X軸方向に押し出す
				if (overlapX1 < overlapX2) {
					playerPos.x += overlapX1; // 右へ押し出す
				} else {
					playerPos.x -= overlapX2; // 左へ押し出す
				}
			} else {
				// Z軸方向に押し出す
				if (overlapZ1 < overlapZ2) {
					playerPos.z += overlapZ1; // 奥へ押し出す
				} else {
					playerPos.z -= overlapZ2; // 手前へ押し出す
				}
			}

			// 押し出した結果をプレイヤーの座標に反映
			player_->SetPosition(playerPos);

			// 複数の壁と連続で当たるケースを考慮し、判定用AABBもその場で更新
			playerAABB.max.x = playerPos.x + 0.4f;
			playerAABB.min.x = playerPos.x - 0.4f;
			playerAABB.max.z = playerPos.z + 0.4f;
			playerAABB.min.z = playerPos.z - 0.4f;
		}
	}

	// ==========================================
	// 敵と壁の判定（めり込み防止 ＆ ノックバック時死亡）
	// ==========================================
	if (!enemy_->IsDead()) {
		AABB enemyAABB = enemy_->GetBodyCol();
		Vector3 enemyPos = enemy_->GetPos();

		for (const auto& wall : wallManager_->GetWalls()) {
			AABB wallAABB = wall->GetCollision();

			// AABB同士の交差判定
			if (enemyAABB.min.x <= wallAABB.max.x && enemyAABB.max.x >= wallAABB.min.x && enemyAABB.min.y <= wallAABB.max.y && enemyAABB.max.y >= wallAABB.min.y && enemyAABB.min.z <= wallAABB.max.z &&
			    enemyAABB.max.z >= wallAABB.min.z) {

				// ----------------------------------------
				// 壁激突死の判定：もしノックバック中なら死亡させて処理を抜ける
				// ----------------------------------------
				if (enemy_->IsKnockBack()) {
					enemy_->Kill();
					break;
				}

				// ----------------------------------------
				// 通常時の押し出し処理
				// ----------------------------------------
				float overlapX1 = wallAABB.max.x - enemyAABB.min.x;
				float overlapX2 = enemyAABB.max.x - wallAABB.min.x;
				float overlapZ1 = wallAABB.max.z - enemyAABB.min.z;
				float overlapZ2 = enemyAABB.max.z - wallAABB.min.z;

				float minOverlapX = std::min(overlapX1, overlapX2);
				float minOverlapZ = std::min(overlapZ1, overlapZ2);

				if (minOverlapX < minOverlapZ) {
					if (overlapX1 < overlapX2) {
						enemyPos.x += overlapX1;
					} else {
						enemyPos.x -= overlapX2;
					}
				} else {
					if (overlapZ1 < overlapZ2) {
						enemyPos.z += overlapZ1;
					} else {
						enemyPos.z -= overlapZ2;
					}
				}

				// 結果を敵の座標に反映
				enemy_->SetPos(enemyPos);

				// 連続で壁に当たるケースを考慮してAABB更新
				enemyAABB.max.x = enemyPos.x + 0.5f;
				enemyAABB.min.x = enemyPos.x - 0.5f;
				enemyAABB.max.z = enemyPos.z + 0.5f;
				enemyAABB.min.z = enemyPos.z - 0.5f;
			}
		}
	}
}
