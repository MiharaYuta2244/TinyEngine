#include "ResultScene.h"
#include "SceneManager.h"
#include <numbers>

void ResultScene::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;
	sceneManager_ = sceneManager;

	// さいかいモデル
	restartModel_ = std::make_unique<Object3d>();
	restartModel_->Initialize(engineContext_);
	restartModel_->SetModel("saikai.obj");
	restartModel_->SetTranslate({5.0f, 0.0f, 0.0f});
	restartModel_->SetScale({5.0f, 5.0f, 5.0f});
	restartModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});

	// たいとるへモデル
	toTitleModel_ = std::make_unique<Object3d>();
	toTitleModel_->Initialize(engineContext_);
	toTitleModel_->SetModel("ToTitle2.obj");
	toTitleModel_->SetTranslate({35.0f, 0.0f, 0.0f});
	toTitleModel_->SetScale({5.0f, 5.0f, 5.0f});
	toTitleModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
}

void ResultScene::Update() {
	restartModel_->Update();
	toTitleModel_->Update();

	bool backInput = keyboard_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;
	if (backInput) {
		sceneManager_->ChangeScene("Title");
	}

#ifdef USE_IMGUI
	ImGui::Begin("Result");
	ImGui::TextWrapped("Result Scene");
	ImGui::TextWrapped("Press Space or GamePad A to Return to Title");
	ImGui::End();
#endif
}

void ResultScene::Draw() {
	restartModel_->Draw();
	toTitleModel_->Draw();
}

void ResultScene::Finalize() {
	restartModel_.reset();
	toTitleModel_.reset();
}
