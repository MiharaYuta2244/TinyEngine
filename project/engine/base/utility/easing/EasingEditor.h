#pragma once
#include "AnimationBundle.h"
#include <array>
#include <string>

/// <summary>
/// イージングエディター
/// </summary>
class EasingEditor {
public:
	// コンストラクタ
	EasingEditor();

	// エディターの更新処理
	void DrawWindow(float deltaTime);

private:
	// EaseTypeを選択する為の処理
	void DrawEaseSelector();

	// グラフの描画
	void DrawCurveGraph();

	// プレビュー描画
	void DrawPreview(float deltaTime);

	// プレビューのリセット
	void ResetPreview();

private:
	int currentEaseIndex_ = 1;

	// アニメーション時間
	float duration_ = 1.0f;

	// プレビュー用のインスタンス
	EasingAnimation<float> previewAnim_;

	// 現在のプレビュー値
	float currentValue_ = 0.0f;

	// Enumと文字列の対応テーブル
	const char* easeNames_[30] = {
	    "EaseInSine",    "EaseOutSine",    "EaseInOutSine", "EaseInQuad",    "EaseOutQuad",    "EaseInOutQuad",  "EaseInCubic",      "EaseOutCubic",  "EaseInOutCubic", "EaseInQuart",
	    "EaseOutQuart",  "EaseInOutQuart", "EaseInQuint",   "EaseOutQuint",  "EaseInOutQuint", "EaseInExpo",     "EaseOutExpo",      "EaseInOutExpo", "EaseInCirc",     "EaseOutCirc",
	    "EaseInOutCirc", "EaseInBack",     "EaseOutBack",   "EaseInOutBack", "EaseInElastic",  "EaseOutElastic", "EaseInOutElastic", "EaseOutBounce", "EaseInBounce",   "EaseInOutBounce",
	};
};
