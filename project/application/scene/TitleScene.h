#pragma once
#include "BaseScene.h"
#include "EndModel.h"
#include "Object3d.h"
#include "Sprite.h"
#include "StartModel.h"
#include "TitleText.h"
#include "StageModel.h"
#include <array>
#include <memory>

class TitleScene : public BaseScene {
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// シーン切り替え処理
	void ChangeScene();

	// タイトルの状態遷移
	void StateChange();

	// タイトル1更新処理
	void Title1Update();

	// タイトル1更新処理
	void Title2Update();

	// タイトル1の描画
	void Title1Draw();

	// タイトル2の描画
	void Title2Draw();

private:
	enum class TitleState { START, END, BACK_SCENE, STAGE1, STAGE2, STAGE3, CHARACTER_SELECT };

	enum class TitleNumber { TITLE1, TITLE2 };

	struct StateTransition {
		TitleState left;
		TitleState right;
		std::function<void()> onDecide;
	};

private:
	// タイトルテキストモデル
	std::unique_ptr<TitleText> titleText_;

	// はじめるモデル
	std::unique_ptr<StartModel> startModel_;

	// おわるモデル
	std::unique_ptr<EndModel> endModel_;

	// ステージ1モデル
	std::unique_ptr<StageModel> stage1Model_;

	// タイトルの状態
	TitleState titleState_ = TitleState::START;

	// タイトルシーン1か2か
	TitleNumber titleNumber_ = TitleNumber::TITLE1;

	// キャラクターセレクト画面かどうか
	bool isCharacterSelection_ = false;
};
