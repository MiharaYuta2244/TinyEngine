#pragma once
#include "BaseScene.h"
#include "Object3d.h"
#include "Sprite.h"
#include "TitleText.h"
#include <memory>
#include <array>

class TitleScene : public BaseScene {
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	void AllModelLoad();

private:
	// タイトルテキストモデル
	std::unique_ptr<TitleText> titleText_;

	// はじめるモデル
	std::unique_ptr<Object3d> startModel_;

	// おわるモデル
	std::unique_ptr<Object3d> endModel_;

	// タイトルテキストの回転
	float titleRotateY_ = 0.0f;

	// 木のアニメーション用
	float t_ = 0.0f;
	const float kTimer = 2.0f;
	float timer_ = 0.0f;
};
