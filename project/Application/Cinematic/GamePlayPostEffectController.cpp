#include "GamePlayPostEffectController.h"
#include "GameObjects/PlayTestConfig.h"
#include "Scene/SceneManager.h"

using namespace TinyEngine;

void GamePlayPostEffectController::Initialize(const SceneContext& ctx) {
	ctx_ = &ctx;
	commonData_ = ctx_->sceneManager->GetCommonData();

	// シーンで使うエフェクトの宣言
	ctx_->engineContext->postEffectPipeline->SetEffects({
	    PostEffectType::Glitch,           // グリッチ
	    PostEffectType::DeathEffect,      // 死亡時エフェクト
	    PostEffectType::Scanline,         // 走査線
	    PostEffectType::BarrelDistortion, // 歪み
	});

	// ビネットの初期設定
	auto* vignette = ctx_->engineContext->postEffectPipeline->GetPass(PostEffectType::Vignette);
	if (vignette) {
		vignette->SetVignetteColor({1, 0, 0, 1});
	}

	glitchTimer_ = 0.0f;
	elapsedTime_ = 0.0f;
	isDeathAnimStarted_ = false;
	isAnimPlaying_ = false;
	barrelDistortionParam_.strength = 0.05f;
	scanlineParam_.scanlineCount = 150.0f;
	scanlineParam_.intensity = 0.7f;
	scanlineParam_.speed = 5.0f;
	scanlineParam_.time = 0.0f;
}

void GamePlayPostEffectController::Update(float deltaTime, bool isPlayerDead) {
	const bool obscure = PlaytestConfig::obscureMode;

	// グリッチノイズの更新
	UpdateGlitch(deltaTime);

	// 走査の更新
	scanlineParam_.time += deltaTime;
	if (auto* scanline = ctx_->engineContext->postEffectPipeline->GetPass(PostEffectType::Scanline)) {
		scanline->SetScanlineParam(scanlineParam_);
	}

	// 歪みの更新
	if (auto* barrel = ctx_->engineContext->postEffectPipeline->GetPass(PostEffectType::BarrelDistortion)) {
		barrel->SetFisheyeParam(barrelDistortionParam_);
	}

	// 死亡時アニメーションの更新
	isAnimPlaying_ = numSamplesAnim_.anim.Update(deltaTime, numSamplesAnim_.temp);

	// エフェクトパスの取得
	auto* deathEffect = ctx_->engineContext->postEffectPipeline->GetPass(PostEffectType::DeathEffect);

	if (!isPlayerDead) {
		// 生存時のエフェクト更新
		if (deathEffect) {
			deathEffect->SetDeathEffectIntensity(0.0f);
		}
	} else {
		// 死亡時のエフェクト更新
		if (deathEffect) {
			deathEffect->SetDeathEffectIntensity(1.0f);
		}
	}
}

void GamePlayPostEffectController::StartDeathAnimation() {
	if (!isDeathAnimStarted_) {
		isDeathAnimStarted_ = true;
		numSamplesAnim_.anim.Start(0.0f, 1.0f, 1.0f, EaseType::EASEOUTCIRC);
	}
}

void GamePlayPostEffectController::UpdateGlitch(float deltaTime) {
	if (glitchTimer_ > 0.0f) {
		glitchTimer_ -= deltaTime;
		elapsedTime_ += deltaTime;
		if (glitchTimer_ < 0.0f) {
			glitchTimer_ = 0.0f;
			elapsedTime_ = 0.0f;
		}
	}

	bool shouldGlitch = !PlaytestConfig::obscureMode && glitchTimer_ > 0.0f;
	float intensity = shouldGlitch ? 1.0f : 0.0f;
	auto* glitch = ctx_->engineContext->postEffectPipeline->GetPass(PostEffectType::Glitch);

	if (glitch) {
		glitch->SetGlitchTime(elapsedTime_);
		glitch->SetGlitchIntensity(intensity);
	}
}

void GamePlayPostEffectController::Finalize() {
	// シーン終了時にエフェクトをデフォルトに戻す
	ctx_->engineContext->postEffectPipeline->SetEffects({PostEffectType::FullScreen});
}