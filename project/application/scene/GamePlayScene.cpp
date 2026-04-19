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
}

void GamePlayScene::Update() {
	// プレイヤーの更新処理
	player_->Update(timeManager_->GetDeltaTime(), keyboard_, enemy_.get());

	// プレイヤーが死亡したらシーン遷移
	if(player_->IsDead()){
		sceneManager_->ChangeScene("Title");
	}

	// 敵の更新処理
	enemy_->Update(timeManager_->GetDeltaTime(), player_->GetPosition(), enemyBulletManager_.get());

	// 敵の弾の更新処理
	enemyBulletManager_->Update(timeManager_->GetDeltaTime());

	// 当たり判定
	CollisionGameObjects();

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
}

void GamePlayScene::Finalize() {}

void GamePlayScene::CollisionGameObjects() {
	// プレイヤーと敵の弾の当たり判定
	Sphere playerSphere = {player_->GetPosition(), 1.0f}; // プレイヤーの当たり判定形状（仮）
	for (const auto& bullet : enemyBulletManager_->GetBullets()) {
		Sphere bulletSphere = {bullet->GetPosition(), 0.5f}; // 弾の当たり判定形状（仮）

		// 衝突判定
		if (Collision::Intersect(playerSphere, bulletSphere)) {
			// プレイヤーにダメージを与える処理
			player_->Damage(1.0f);
		}
	}

	// プレイヤーの攻撃用範囲と敵の当たり判定
	if (Collision::Intersect(player_->GetAttackCol(), enemy_->GetAABBCol())) {
		// プレイヤーの攻撃フラグを立てる
		player_->SetEnableAttack(true);
	}else {
		// プレイヤーの攻撃フラグを下す
		player_->SetEnableAttack(false);
	}
}
