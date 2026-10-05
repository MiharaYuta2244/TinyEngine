#include "FadeManager.h"
#include "FadeStates.h"
#include "Random.h"
#include <Easing.h>
#include <algorithm>

using namespace TinyEngine;

void FadeManager::Initialize(EngineContext* ctx) {
	for (int i = 0; i < kStripeCount; ++i) {
		// 各帯のベースとなる遅延時間を記録
		stripeDelays_[i] = RandomUtils::RangeFloat(0.0f, 0.2f);

		for (int j = 0; j < kLayerCount; ++j) {
			fadeSprites_[i][j] = std::make_unique<Sprite>();
			fadeSprites_[i][j]->Initialize(ctx, "white.png");
			fadeSprites_[i][j]->SetAnchorPoint({0.0f, 0.0f});
			float width = 1280.0f / kStripeCount;
			fadeSprites_[i][j]->SetPosition({i * width, 0.0f});
			fadeSprites_[i][j]->SetSize({width, 0.0f});

			// 設定されたレイヤーの色を適用
			fadeSprites_[i][j]->SetColor(layerColors_[j]);
		}
	}

	ChangeState(std::make_unique<FadeStateNone>());
}

void FadeManager::Update(float deltaTime) {
	if (currentState_) {
		currentState_->Update(this, deltaTime);
	}
}

void FadeManager::Draw() {
	for (int i = 0; i < kStripeCount; ++i) {
		for (int j = 0; j < kLayerCount; ++j) {
			if (fadeSprites_[i][j]) {
				fadeSprites_[i][j]->Update();
				fadeSprites_[i][j]->Draw();
			}
		}
	}
}

void FadeManager::FadeOutTo(const std::string& sceneName) {
	if (!sceneName.empty()) {
		requestedSceneName_ = sceneName;
		ChangeState(std::make_unique<FadeStateFadeOut>());
	}
}

void FadeManager::NotifySceneChanged() {
	isWaitingForSceneChange_ = false;
	requestedSceneName_ = "";
	ChangeState(std::make_unique<FadeStateFadeIn>());
}

void FadeManager::ChangeState(std::unique_ptr<IFadeState> newState) {
	if (currentState_) {
		currentState_->Exit(this);
	}
	currentState_ = std::move(newState);
	if (currentState_) {
		currentState_->Enter(this);
	}
}

void FadeManager::SetFadeAlpha(float progress) {
	bool isFadeOut = !requestedSceneName_.empty() || isWaitingForSceneChange_;
	float layerDelayGap = 0.1f; // レイヤー間の遅延差

	// アニメーション全体がduration内に収まるように最大遅延を計算
	float maxDelay = 0.2f + (kLayerCount - 1) * layerDelayGap;
	float duration = 1.0f - maxDelay;

	for (int i = 0; i < kStripeCount; ++i) {
		for (int j = 0; j < kLayerCount; ++j) {
			// フェードアウト：背面から動かし、前面を遅らせて伸ばす
			// フェードイン：前面から動かし、背面を遅らせて縮める
			float delayOffset = isFadeOut ? (j * layerDelayGap) : ((kLayerCount - 1 - j) * layerDelayGap);
			float delay = stripeDelays_[i] + delayOffset;

			// 遅延を考慮した個別の進行度を計算
			float localProgress = (progress - delay) / duration;
			localProgress = std::clamp(localProgress, 0.0f, 1.0f);

			// イージング
			float easeProgress = Easing::ApplyEasing(EaseType::EASEOUTCUBIC, localProgress);

			float width = 1280.0f / kStripeCount;
			float x = i * width;
			float currentHeight = 720.0f * easeProgress;

			// インデックスが偶数の帯は上部から、奇数の帯は下部から動かす
			bool isTop = (i % 2 == 0);

			if (isFadeOut) {
				// フェードアウト
				if (isTop) {
					fadeSprites_[i][j]->SetPosition({x, 0.0f});
					fadeSprites_[i][j]->SetSize({width, currentHeight});
				} else {
					fadeSprites_[i][j]->SetPosition({x, 720.0f - currentHeight});
					fadeSprites_[i][j]->SetSize({width, currentHeight});
				}
			} else {
				// フェードイン
				if (isTop) {
					fadeSprites_[i][j]->SetPosition({x, 720.0f - currentHeight});
					fadeSprites_[i][j]->SetSize({width, currentHeight});
				} else {
					fadeSprites_[i][j]->SetPosition({x, 0.0f});
					fadeSprites_[i][j]->SetSize({width, currentHeight});
				}
			}
		}
	}
}

void FadeManager::SetLayerColor(size_t layerIndex, const Vector4& color) {
	if (layerIndex >= kLayerCount)
		return;

	layerColors_[layerIndex] = color;

	// 既にスプライトが生成されていれば色を即座に更新
	for (int i = 0; i < kStripeCount; ++i) {
		if (fadeSprites_[i][layerIndex]) {
			fadeSprites_[i][layerIndex]->SetColor(color);
		}
	}
}

void FadeManager::SetLayerColor(size_t layerIndex, float r, float g, float b, float a) { SetLayerColor(layerIndex, Vector4{r, g, b, a}); }

void FadeManager::SetLayerColors(const Vector4& color0, const Vector4& color1, const Vector4& color2) {
	SetLayerColor(0, color0);
	SetLayerColor(1, color1);
	SetLayerColor(2, color2);
}