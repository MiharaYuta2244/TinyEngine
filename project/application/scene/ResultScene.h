#pragma once
#include "BaseScene.h"
#include "Object3d.h"
#include <memory>

class ResultScene : public BaseScene {
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// さいかいモデル
	std::unique_ptr<Object3d> restartModel_;

	// たいとるへモデル
	std::unique_ptr<Object3d> toTitleModel_;
};
