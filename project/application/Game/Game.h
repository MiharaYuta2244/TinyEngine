#pragma once
#include "Framework.h"
#include "SceneManager.h"
#include "Sprite.h"
#include "Fade/FadeManager.h"
#include <memory>

class Game : public Framework {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// シーンマネージャー
	std::unique_ptr<SceneManager> sceneManager_ = std::make_unique<SceneManager>();

	// フェードマネージャー
	std::unique_ptr<FadeManager> fadeManager_;
};