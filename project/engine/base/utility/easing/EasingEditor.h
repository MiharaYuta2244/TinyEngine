#pragma once
#include "AnimationBundle.h"
#include "Vector3.h"
#include <array>
#include <string>

// 対象を選択するための列挙型
enum class TargetType{
	SCALE,
	ROTATE,
	TRANSLATE
};

/// <summary>
/// イージングエディター
/// </summary>
class EasingEditor {
public:
	// コンストラクタ
	EasingEditor();

	// エディターの更新処理
	void DrawWindow(float deltaTime);

	// Getter
	bool IsPlaying()const{return objectAnim_.GetIsActive();}
	Vector3 GetCurrentObjectValue() const { return currentObjectValue_; }
	TargetType GetTargetType() const { return static_cast<TargetType>(currentTargetIndex_); }

private:
	// EaseTypeを選択する為の処理
	void DrawEaseSelector();

	// グラフの描画
	void DrawCurveGraph();

	// プレビュー描画
	void DrawPreview(float deltaTime);

	// プレビューのリセット
	void ResetPreview();

	// CSV保存関数
	void SaveToCSV(const std::string& filename);

	// CSVロード関数
	void LoadFromCSV(const std::string& filename);

private:
	int currentEaseIndex_ = 1;

	// アニメーション時間
	float duration_ = 1.0f;

	// プレビュー用のインスタンス
	EasingAnimation<float> previewAnim_;

	// 現在のプレビュー値
	float currentValue_ = 0.0f;

	// 現在のターゲット(SRT)
	int currentTargetIndex_ = 2;

	// 開始値
	Vector3 startValue_ = {0.0f,0.0f,0.0f};

	// 終了値
	Vector3 endValue_ = {0.0f,0.0f,0.0f};

	// 現在値
	Vector3 currentObjectValue_ = {0.0f,0.0f,0.0f};

	// プレビュー用のインスタンス
	EasingAnimation<Vector3> objectAnim_;

	// ターゲットの文字列の対応テーブル
	const char* targetNames_[3] = {"Scale","Rotate","Translate"};

	// Enumと文字列の対応テーブル
	const char* easeNames_[30] = {
	    "EaseInSine",    "EaseOutSine",    "EaseInOutSine", "EaseInQuad",    "EaseOutQuad",    "EaseInOutQuad",  "EaseInCubic",      "EaseOutCubic",  "EaseInOutCubic", "EaseInQuart",
	    "EaseOutQuart",  "EaseInOutQuart", "EaseInQuint",   "EaseOutQuint",  "EaseInOutQuint", "EaseInExpo",     "EaseOutExpo",      "EaseInOutExpo", "EaseInCirc",     "EaseOutCirc",
	    "EaseInOutCirc", "EaseInBack",     "EaseOutBack",   "EaseInOutBack", "EaseInElastic",  "EaseOutElastic", "EaseInOutElastic", "EaseOutBounce", "EaseInBounce",   "EaseInOutBounce",
	};
};
