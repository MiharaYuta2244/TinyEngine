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
	player_ = std::make_unique<Player::Render>();
	player_->Initialize(ctx, keyboard);

	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(ctx, "white.png");
	sprite_->SetSize({100.0f, 100.0f});
}

void GamePlayScene::Update() {
	player_->Update();
	sprite_->Update();

#ifdef USE_IMGUI
	ImGui::Begin("Camera");
	ImGui::DragFloat3("Rotate", &debugCamera_->GetRotate().x, 0.01f);
	ImGui::DragFloat3("Translate", &debugCamera_->GetTranslation().x, 0.01f);
	ImGui::End();

	ImGui::Begin("Sprite");
	ImGui::DragFloat2("Position", &sprite_->GetPosition().x, 1.0f);
	ImGui::DragFloat2("Size", &sprite_->GetSize().x, 1.0f);
	ImGui::End();
#endif // USE_IMGUI
}

void GamePlayScene::Draw() {
	player_->Draw();
	sprite_->Draw();
}

void GamePlayScene::Finalize() {}