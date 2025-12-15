#include "TitleScene.h"
#include "SceneManager.h"
#include "Easing.h"
#include <numbers>
#include <algorithm>

void TitleScene::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;
	sceneManager_ = sceneManager;

	// モデルの読み込み
	AllModelLoad();

	// タイトルテキストモデル
	titleText_ = std::make_unique<TitleText>();
	titleText_->Initialize(engineContext_);

	// はじめるモデル
	startModel_ = std::make_unique<Object3d>();
	startModel_->Initialize(engineContext_);
	startModel_->SetModel("start.obj");
	startModel_->SetTranslate({20.0f, 2.0f, 0.0f});
	startModel_->SetScale({5.0f, 5.0f, 5.0f});
	startModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});

	// おわるモデル
	endModel_ = std::make_unique<Object3d>();
	endModel_->Initialize(engineContext_);
	endModel_->SetModel("end.obj");
	endModel_->SetTranslate({20.0f, -2.0f, 0.0f});
	endModel_->SetScale({5.0f, 5.0f, 5.0f});
	endModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});

	// アニメーション用の変数をリセット
	timer_ = 0.0f;
	t_ = 0.0f;
}

void TitleScene::Update() {
	// 木のモデルアニメーション
	timer_ += timeManager_->GetDeltaTime();
	t_ = timer_ / kTimer;
	t_ = std::clamp(t_, 0.0f, 1.0f);

	float x = Easing::easeInOutBack(t_) * 5.0f;

	titleText_->Update(timeManager_->GetDeltaTime());
	startModel_->Update();
	endModel_->Update();

	// ゲーム開始入力
	bool startInput = keyboard_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;
	if (startInput) {
		sceneManager_->ChangeScene("GamePlay");
	}
}

void TitleScene::Draw() {
	titleText_->Draw();
	startModel_->Draw();
	endModel_->Draw();
}

void TitleScene::Finalize() {
	titleText_.reset();
	startModel_.reset();
	endModel_.reset();
}

void TitleScene::AllModelLoad() {
	engineContext_->modelManager->LoadModel("fence.obj");
	engineContext_->modelManager->LoadModel("plane.obj");
	engineContext_->modelManager->LoadModel("axis.obj");
	engineContext_->modelManager->LoadModel("SkySphere.obj");
	engineContext_->modelManager->LoadModel("skydome.obj");
	engineContext_->modelManager->LoadModel("Field.obj");
	engineContext_->modelManager->LoadModel("sphere.obj");
	engineContext_->modelManager->LoadModel("Box.obj");
	engineContext_->modelManager->LoadModel("Boss.obj");
	engineContext_->modelManager->LoadModel("Hiyoko.obj");
	engineContext_->modelManager->LoadModel("HiyokoGlass.obj");
	engineContext_->modelManager->LoadModel("HiyokoStudent.obj");
	engineContext_->modelManager->LoadModel("HiyokoAfro.obj");
	engineContext_->modelManager->LoadModel("Title.obj");
	engineContext_->modelManager->LoadModel("HiyokoPropeller.obj");
	engineContext_->modelManager->LoadModel("ClimbDrop.obj");
	engineContext_->modelManager->LoadModel("tree.obj");
	engineContext_->modelManager->LoadModel("saikai.obj");
	engineContext_->modelManager->LoadModel("ToTitle2.obj");
	engineContext_->modelManager->LoadModel("start.obj");
	engineContext_->modelManager->LoadModel("end.obj");
}
