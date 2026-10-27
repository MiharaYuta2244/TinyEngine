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
	// 初期化関数
	void Initialize(const SceneContext& ctx) override;

	// 更新関数
	void Update() override;

	// 描画関数
	void Draw() override;

	// 終了時処理
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
