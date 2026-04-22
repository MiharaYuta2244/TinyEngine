#pragma once
#include "BaseScene.h"

class TitleScene : public BaseScene{
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, Camera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:

};
