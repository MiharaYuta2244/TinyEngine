#pragma once
#include "EngineContext.h"
#include "Font.h"
#include "GameObjects/ResultUI/Animation/SpriteScaleWipeAnimator.h"
#include "GameTimer.h"
#include "Sprite.h"
#include "TextSprite.h"
#include <memory>
#include <string>

/// <summary>
/// リザルトシーンでのスコア表示クラス
/// </summary>
class ResultScoreDisplay {
public:
	// 初期化
	void Initialize(EngineContext* ctx, bool isClear, float clearTime, int killCount);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

private:
	// スプライト生成ヘルパー関数
	std::unique_ptr<TinyEngine::Sprite> CreateDigitSprite(EngineContext* ctx, const std::string& textureName, const Vector2& pos);
	std::unique_ptr<TinyEngine::Sprite> CreateSymbolSprite(EngineContext* ctx, const std::string& textureName, const Vector2& pos);

private:
	// フォント
	std::unique_ptr<TinyEngine::Font> font_;

	// 表示テキスト
	std::unique_ptr<TinyEngine::TextSprite> timeText_;      // クリアタイマー
	std::unique_ptr<TinyEngine::TextSprite> killCountText_; // キル数

	// フォント設定
	const std::wstring kFontFamily = L"Dela Gothic One";
	const int kFontPixelHeight = 80;
	const std::wstring kCharSet = L"0123456789:-体";

	// 表示位置(テキストの左上基準)
	const Vector2 kTimeBasePos = {490.0f, 300.0f};
	const Vector2 kKillCountBasePos = {490.0f, 400.0f};

	// ワイプアニメーション用
	std::unique_ptr<SpriteScaleWipeAnimator> timerWipe_;
	std::unique_ptr<SpriteScaleWipeAnimator> killCountWipe_;

	// 時間差再生用タイマー
	float killCountWipeDelay_ = 0.2f; // タイマー側のワイプから少し遅れて開始する
	bool isKillCountWipeStarted_ = false;

	// ワイプアニメーション開始までのタイマー
	GameTimer wipeAnimStartTimer_;
};