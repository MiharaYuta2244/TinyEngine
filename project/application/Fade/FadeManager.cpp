#include "FadeManager.h"
#include "FadeStates.h"
#include "Random.h"
#include <Easing.h>
#include <algorithm>

using namespace TinyEngine;

void FadeManager::Initialize(EngineContext* ctx) {
	for (int i = 0; i < kStripeCount; ++i) {
		fadeSprites_[i] = std::make_unique<Sprite>();
		fadeSprites_[i]->Initialize(ctx, "white.png");
		fadeSprites_[i]->SetAnchorPoint({0.0f, 0.0f});
		float width = 1280.0f / kStripeCount;
		fadeSprites_[i]->SetPosition({i * width, 0.0f});
		fadeSprites_[i]->SetSize({width, 0.0f});
		fadeSprites_[i]->SetColor({0.0f, 0.0f, 0.0f, 1.0f});

		// 各帯の遅延時間を記録
		stripeDelays_[i] = RandomUtils::RangeFloat(0.0f, 0.4f);
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
		if (fadeSprites_[i]) {
			fadeSprites_[i]->Update();
			fadeSprites_[i]->Draw();
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

	float maxDelay = 0.4f;
	float duration = 1.0f - maxDelay;

	for (int i = 0; i < kStripeCount; ++i) {
		float delay = stripeDelays_[i];

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
				// 上端固定で、下へ向かって伸びる
				fadeSprites_[i]->SetPosition({x, 0.0f});
				fadeSprites_[i]->SetSize({width, currentHeight});
			} else {
				// 下端固定で、上へ向かって伸びる
				fadeSprites_[i]->SetPosition({x, 720.0f - currentHeight});
				fadeSprites_[i]->SetSize({width, currentHeight});
			}
		} else {
			// フェードイン
			if (isTop) {
				// 下へ向かって抜けていく
				fadeSprites_[i]->SetPosition({x, 720.0f - currentHeight});
				fadeSprites_[i]->SetSize({width, currentHeight});
			} else {
				// 上へ向かって抜けていく
				fadeSprites_[i]->SetPosition({x, 0.0f});
				fadeSprites_[i]->SetSize({width, currentHeight});
			}
		}
	}
}
