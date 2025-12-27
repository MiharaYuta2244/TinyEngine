#include "TitleScene.h"
#include "Easing.h"
#include "SceneManager.h"
#include <algorithm>
#include <numbers>

void TitleScene::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;
	sceneManager_ = sceneManager;

	// タイトルテキストモデル
	titleText_ = std::make_unique<TitleText>();
	titleText_->Initialize(engineContext_);

	// はじめるモデル
	startModel_ = std::make_unique<StartModel>();
	startModel_->Initialize(engineContext_);

	// おわるモデル
	endModel_ = std::make_unique<EndModel>();
	endModel_->Initialize(engineContext_);

	// ステージ1モデル
	stage1Model_ = std::make_unique<StageModel>();
	stage1Model_->Initialize(engineContext_);

	// タイトルの状態
	titleState_ = TitleState::START;

	// タイトルシーン1か2か
	titleNumber_ = TitleNumber::TITLE1;
}

void TitleScene::Update() {
	// モデルの更新　タイトル1
	Title1Update();

	// モデルの更新　タイトル2
	Title2Update();

	// タイトルの状態切り替え
	StateChange();

	// シーン切り替え処理
	ChangeScene();
}

void TitleScene::Draw() {
	// タイトル番号が1の時
	Title1Draw();

	// タイトル番号が2の時
	Title2Draw();
}

void TitleScene::Finalize() {
	titleText_.reset();
	startModel_.reset();
	endModel_.reset();
}

void TitleScene::ChangeScene() {
	if (titleNumber_ == TitleNumber::TITLE2)
		return;

	// 入力
	bool upInput = keyboard_->KeyTriggered(DIK_W) || gamePad_->GetState().buttonsPressed.dpadUp;
	bool downInput = keyboard_->KeyTriggered(DIK_S) || gamePad_->GetState().buttonsPressed.dpadDown;

	// 上入力の場合
	if (upInput) {
		titleState_ = TitleState::START; // ゲーム開始
		startModel_->SetSelected(true);  // 選択状態にする
		endModel_->SetSelected(false);   // 選択状態を解除
	}

	// 下入力の場合
	if (downInput) {
		titleState_ = TitleState::END;   // ゲーム終了
		endModel_->SetSelected(true);    // 選択状態にする
		startModel_->SetSelected(false); // 選択状態を解除
	}

	// ゲーム開始入力
	bool startInput = keyboard_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;
	if (startInput && titleState_ == TitleState::START) {
		titleNumber_ = TitleNumber::TITLE2;   // タイトル2へ
		titleState_ = TitleState::BACK_SCENE; // タイトルの状態をバックシーンに変更
	} else if (startInput && titleState_ == TitleState::END) {
		PostQuitMessage(0); // ゲーム終了
	}
}

void TitleScene::StateChange() {
	if (titleNumber_ == TitleNumber::TITLE1)
		return;

	// 入力
	bool upInput = keyboard_->KeyTriggered(DIK_W) || gamePad_->GetState().buttonsPressed.dpadUp;
	bool downInput = keyboard_->KeyTriggered(DIK_S) || gamePad_->GetState().buttonsPressed.dpadDown;
	bool rightInput = keyboard_->KeyTriggered(DIK_D) || gamePad_->GetState().buttonsPressed.dpadRight;
	bool leftInput = keyboard_->KeyTriggered(DIK_A) || gamePad_->GetState().buttonsPressed.dpadLeft;
	bool decideInput = keyboard_->KeyTriggered(DIK_SPACE) || gamePad_->GetState().buttonsPressed.a;

	// 状態遷移テーブル
	static const std::unordered_map<TitleState, StateTransition> transitions = {
	    {TitleState::BACK_SCENE,
	     {TitleState::CHARACTER_SELECT,           // left
	      TitleState::STAGE1,                     // right
	      [this]() {                              // decide
		      keyboard_->Reset();                 // 入力状態をリセット
		      titleNumber_ = TitleNumber::TITLE1; // タイトル番号を元に戻す
		      titleState_ = TitleState::START;    // タイトルの状態も初期状態元に戻す
	      }}	                                                                                                                               },
	    {TitleState::STAGE1,           {TitleState::BACK_SCENE, TitleState::STAGE2, [this]() { sceneManager_->ChangeScene("GamePlay"); }}      }, // ステージ1へ
	    {TitleState::STAGE2,           {TitleState::STAGE1, TitleState::STAGE3, [this]() { sceneManager_->ChangeScene("GamePlay"); }}          }, // ステージ2へ
	    {TitleState::STAGE3,           {TitleState::STAGE2, TitleState::CHARACTER_SELECT, [this]() { sceneManager_->ChangeScene("GamePlay"); }}}, // ステージ3へ
	    {TitleState::CHARACTER_SELECT, {TitleState::STAGE3, TitleState::BACK_SCENE, [this]() { isCharacterSelection_ = true; }}                }, // キャラクターセレクトへ
	};

	const auto& t = transitions.at(titleState_);

	if (leftInput)
		titleState_ = t.left;
	if (rightInput)
		titleState_ = t.right;
	if (decideInput)
		t.onDecide();
}

void TitleScene::Title1Update() {
	if (titleNumber_ == TitleNumber::TITLE1) {
		titleText_->Update(timeManager_->GetDeltaTime());
		startModel_->Update(timeManager_->GetDeltaTime());
		endModel_->Update(timeManager_->GetDeltaTime());
	}
}

void TitleScene::Title2Update() {
	if (titleNumber_ == TitleNumber::TITLE2) {
		stage1Model_->Update(timeManager_->GetDeltaTime());
	}
}

void TitleScene::Title1Draw() {
	if (titleNumber_ == TitleNumber::TITLE1) {
		titleText_->Draw();
		startModel_->Draw();
		endModel_->Draw();
	}
}

void TitleScene::Title2Draw() {
	if (titleNumber_ == TitleNumber::TITLE2) {
		stage1Model_->Draw();
	}
}
