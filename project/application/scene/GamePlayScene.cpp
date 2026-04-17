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
}

void GamePlayScene::Update() {
	// プレイヤーの更新処理
	player_->Update(timeManager_->GetDeltaTime(), keyboard_);

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
}

void GamePlayScene::Finalize() {}