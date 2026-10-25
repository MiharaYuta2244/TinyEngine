#include "FadeManager.h"
#include "FadeStates.h"
#include "Random.h"
#include <Easing.h>
#include <algorithm>

using namespace TinyEngine;

namespace {
constexpr float kW = 1280.0f, kH = 720.0f;
const Vector4 kCoverColor = {0.16f, 0.16f, 0.16f, 1.0f}; // 暗転の背景色
constexpr float kLineHeight = 120.0f;                     // 横ラインの太さ
constexpr float kBarWidth = 120.0f;                       // 縦バーの太さ
constexpr float kBarHeight = 1000.0f;
constexpr float kBarAngle = 0.17f; // 縦バーの傾き
constexpr const char* kLogoTexture = "Title_Logo.png";

float Phase(float t, float s, float e) { return std::clamp((t - s) / (e - s), 0.0f, 1.0f); }
float EaseOut(float t) { return Easing::ApplyEasing(EaseType::EASEOUTCUBIC, t); }
void SetRect(TinyEngine::Sprite* s, float x, float y, float w, float h) {
	s->SetPosition({x, y});
	s->SetSize({w, h});
}
} // namespace

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
	InitializeLogoStyle(ctx);
}

void FadeManager::Update(float deltaTime) {
	if (currentState_) {
		currentState_->Update(this, deltaTime);
	}
}

void FadeManager::Draw() {
	if (style_ == FadeStyle::Logo) {
		DrawLogoStyle();
		return;
	}

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

	if (style_ == FadeStyle::Logo) {
		if (isFadeOut) {
			UpdateLogoOut(progress);
		} else {
			UpdateLogoIn(1.0f - progress);
		}

		return;
	}

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

void FadeManager::InitializeLogoStyle(EngineContext* ctx) {
	auto makeRect = [&](std::unique_ptr<Sprite>& s, const Vector4& color, const Vector2& anchor) {
		s = std::make_unique<Sprite>();
		s->Initialize(ctx, "white.png");
		s->SetAnchorPoint(anchor);
		s->SetColor(color);
		s->SetSize({0.0f, 0.0f});
	};

	makeRect(coverL_, kCoverColor, {0.0f, 0.0f});
	makeRect(coverR_, kCoverColor, {0.0f, 0.0f});
	makeRect(line_, {1, 1, 1, 1}, {0.0f, 0.0f});
	makeRect(bar_, {1, 1, 1, 1}, {0.5f, 0.5f});
	bar_->SetRotation(kBarAngle);

	logo_ = std::make_unique<Sprite>();
	logo_->Initialize(ctx, kLogoTexture);
	logo_->SetAnchorPoint({0.0f, 0.0f});
	logoTexSize_ = logo_->GetSize(); // Initialize直後はテクスチャサイズ
}

void FadeManager::UpdateLogoOut(float t) {
	logoActive_ = true;

	// 左右からグレーのパネルが閉じる
	float half = kW * 0.5f * EaseOut(Phase(t, 0.0f, 0.2f));
	SetRect(coverL_.get(), 0.0f, 0.0f, half, kH);
	SetRect(coverR_.get(), kW - half, 0.0f, half, kH);

	// 白い横ラインが左→右へ伸び、後半で右端へ縮む
	float lineLeft = kW * EaseOut(Phase(t, 0.5f, 0.7f));
	float lineRight = kW * EaseOut(Phase(t, 0.2f, 0.45f));
	float lineW = std::max(lineRight - lineLeft, 0.0f);
	SetRect(line_.get(), lineLeft, kH * 0.5f - kLineHeight * 0.5f, lineW, kLineHeight);
	lineShown_ = lineW > 0.5f;

	// 傾いた縦バーが右→左へ走り、通過した位置から右側のロゴが見えていく
	float sweep = Phase(t, 0.45f, 0.85f);
	float barX = (kW + 150.0f) + (-150.0f - (kW + 150.0f)) * EaseOut(sweep);
	bar_->SetPosition({barX, kH * 0.5f});
	bar_->SetSize({kBarWidth, kBarHeight});
	barShown_ = sweep > 0.0f && sweep < 1.0f;
	ApplyLogoCrop(barX, 1.0f);
}

void FadeManager::UpdateLogoIn(float r) {
	logoActive_ = r < 1.0f;
	lineShown_ = false;
	barShown_ = false;

	ApplyLogoCrop(-kW, 1.0f - Phase(r, 0.3f, 0.6f));

	float left = kW * EaseOut(Phase(r, 0.45f, 1.0f));
	SetRect(coverL_.get(), 0.0f, 0.0f, 0.0f, kH);
	SetRect(coverR_.get(), left, 0.0f, kW - left, kH);
}

void FadeManager::ApplyLogoCrop(float visibleLeft, float alpha) {
	const float dispW = logoTexSize_.x * logoScale_;
	const float dispH = logoTexSize_.y * logoScale_;
	const float left = (kW - dispW) * 0.5f;
	const float top = (kH - dispH) * 0.5f;
	const float right = left + dispW;

	float vl = std::clamp(visibleLeft, left, right);
	float vw = right - vl;
	logoShown_ = vw > 0.5f && alpha > 0.0f;
	if (!logoShown_) {
		return;
	}

	logo_->SetPosition({vl, top});
	logo_->SetSize({vw, dispH});
	logo_->SetTextureLeftTop({(vl - left) / logoScale_, 0.0f});
	logo_->SetTextureSize({vw / logoScale_, logoTexSize_.y});
	logo_->SetColor({1.0f, 1.0f, 1.0f, alpha});
}

void FadeManager::DrawLogoStyle() {
	if (!logoActive_) {
		return;
	}

	coverL_->Update();
	coverL_->Draw();
	coverR_->Update();
	coverR_->Draw();

	if (logoShown_) {
		logo_->Update();
		logo_->Draw();
	}

	if (lineShown_) {
		line_->Update();
		line_->Draw();
	}

	if (barShown_) {
		bar_->Update();
		bar_->Draw();
	}
}