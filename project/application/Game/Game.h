#pragma once
#include "Framework.h"
#include "SceneManager.h"
#include "Fade/FadeManager.h"
#include "Fade/SceneTransition.h"
#include <memory>

class Game : public Framework {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// ポストエフェクトの設定を毎フレーム反映
	void ApplyPostEffectSettings();

private:
	// シーンマネージャー
	std::unique_ptr<SceneManager> sceneManager_;

	// フェードマネージャー
	std::unique_ptr<FadeManager> fadeManager_;

	// シーン切り替えインスタンス
	std::unique_ptr<SceneTransition> sceneTransition_;
};