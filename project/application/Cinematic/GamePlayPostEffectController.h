#pragma once
#include "AnimationBundle.h"
#include "Matrix4x4.h"
#include "PostEffectTypes.h"

// シーンコンテキスト等の前方宣言
struct SceneContext;
struct CommonData;

/// <summary>
/// ゲームプレイ中のポストエフェクト演出を管理するクラス
/// </summary>
class GamePlayPostEffectController {
public:
	// 初期化処理
	void Initialize(const SceneContext& ctx);

	// 更新処理
	void Update(float deltaTime, bool isPlayerDead);

	void Finalize();

	// 死亡演出の開始
	void StartDeathAnimation();

	// 死亡演出が再生中かどうか
	bool GetIsDeathAnimPlaying() const { return isAnimPlaying_; }

	// 当たり判定マネージャに渡すためのタイマー参照
	float& GetGlitchTimer() { return glitchTimer_; }

private:
	void UpdateGlitch(float deltaTime);

private:
	const SceneContext* ctx_ = nullptr;
	CommonData* commonData_ = nullptr;

	// グリッチノイズ用変数
	float glitchTimer_ = 0.0f;
	float elapsedTime_ = 0.0f;
	const float kGlitchDuration = 0.1f;

	// プレイヤー死亡時演出用RadialBlurのNumSamplesAnim
	AnimationBundle<float> numSamplesAnim_;
	bool isDeathAnimStarted_ = false;
	bool isAnimPlaying_ = false;

	ScanlineParam scanlineParam_;                 // 走査線
	BarrelDistortionParam barrelDistortionParam_; // 歪み
};