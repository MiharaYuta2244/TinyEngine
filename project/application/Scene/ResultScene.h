#pragma once
#include "AudioManager.h"
#include "BaseScene.h"
#include "Font.h"
#include "Menu/ResultScoreDisplay.h"
#include "TextSprite.h"
#include "GameObjects/ResultUI/Button/ButtonGroup.h"

/// <summary>
/// リザルトシーン
/// </summary>
class ResultScene : public BaseScene {
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// メインカメラ
	std::unique_ptr<Camera> mainCamera_;

	// ポストエフェクトパラメータ
	ScanlineParam scanlineParam_;                 // 走査線
	BarrelDistortionParam barrelDistortionParam_; // 歪み
	GlitchParam glitchParam_;                     // グリッチ

	// オーディオマネージャーインスタンス
	std::unique_ptr<TinyEngine::AudioManager> audioManager_;

	// フォント
	std::unique_ptr<TinyEngine::Font> font_;
	std::unique_ptr<TinyEngine::Font> titleFont_; // タイトルロゴ用

	// リザルトテキスト
	std::unique_ptr<TinyEngine::TextSprite> resultText_;

	// スコア表示インスタンス
	std::unique_ptr<ResultScoreDisplay> scoreDisplay_;

	// ボタングループ
	std::unique_ptr<ButtonGroup> buttons_;
};
