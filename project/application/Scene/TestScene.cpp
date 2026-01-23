#include "TestScene.h"

void TestScene::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;
	sceneManager_ = sceneManager;

	debugCamera_->SetTranslation({0.0f, 0.0f, -10.0f});

	// 球モデル生成&初期化
	sphereModel_ = std::make_unique<Object3d>();
	sphereModel_->Initialize(ctx);
	sphereModel_->SetModel("MonsterBall.obj");
}

void TestScene::Update() {
	sphereModel_->Update();

#ifdef USE_IMGUI
	ImGui::Begin("Light");
	ImGui::DragFloat3("Direction", &sphereModel_->GetDirectionalLight().direction.x, 0.01f);
	ImGui::DragFloat("Intensity", &sphereModel_->GetDirectionalLight().intensity, 0.01f);
	ImGui::ColorEdit4("Color", &sphereModel_->GetDirectionalLight().color.x);
	ImGui::End();
#endif
}

void TestScene::Draw() { sphereModel_->Draw(); }

void TestScene::Finalize() {}
