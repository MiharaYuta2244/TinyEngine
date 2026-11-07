#pragma once
#include "EngineContext.h"
#include "Font.h"
#include "Sprite.h"
#include "TextSprite.h"
#include <array>
#include <memory>

/// <summary>
/// ステージクリア時のテキストカットイン演出
/// </summary>
class StageClearCutIn {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, int stageNo);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw();

	// 演出の開始処理
	void Trigger();

	// 開始済みかどうか
	bool IsTriggered() const { return isTriggered_; }

	// 演出が終了したかどうか
	bool IsFinished() const { return isTriggered_ && time_ >= totalDuration_; }

private:
	struct Element {
		std::unique_ptr<TinyEngine::TextSprite> text;
		Vector2 basePos{};     // 最終的な表示位置
		Vector2 inOffset{};    // 登場開始位置のずれ
		Vector2 outOffset{};   // 退場先のずれ
		Vector2 driftPerSec{}; // 表示中にゆっくり流れる量
		float delay = 0.0f;    // 登場・退場のずらし時間
		float startScale = 1.0f;
		Vector4 color{1, 1, 1, 1};
	};

	struct Band {
		std::unique_ptr<TinyEngine::Sprite> sprite;
		float delay = 0.0f;
		float height = 360.0f;
		Vector4 color{1, 1, 1, 1};
	};

private:
	std::unique_ptr<TinyEngine::Font> font_;       // 「Stage」「Clear」用
	std::unique_ptr<TinyEngine::Font> numberFont_; // ステージ数用
	std::array<Element, 3> elements_;              // Stage,Clear,ステージ数
	std::array<Band, 2> bands_;                    // 背面の帯

	bool isTriggered_ = false;
	float time_ = 0.0f;

	// タイムライン
	const float kInDuration_ = 0.5f;
	const float kHoldDuration_ = 1.0f;
	const float kOutDuration_ = 0.4f;
	float totalDuration_ = 0.0f;

	float rotation_ = -0.08f; // 全体の傾き
};