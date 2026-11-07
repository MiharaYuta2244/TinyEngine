#include "StageClearCutIn.h"
#include "ColorPalette.h"
#include "Easing.h"
#include "MathUtility.h"
#include <algorithm>
#include <string>

using namespace TinyEngine;

namespace {
float Ease(EaseType type, float t) { return Easing::ApplyEasing(type, std::clamp(t, 0.0f, 1.0f)); }
float Progress(float time, float start, float duration) { return std::clamp((time - start) / duration, 0.0f, 1.0f); }
} // namespace

void StageClearCutIn::Initialize(EngineContext* ctx, int stageNo) {
	font_ = std::make_unique<Font>();
	font_->Initialize(ctx, L"Dela Gothic One", 160, L"StageClear");
	numberFont_ = std::make_unique<Font>();
	numberFont_->Initialize(ctx, L"Dela Gothic One", 360, L"0123456789");

	const Vector4 textColor = ColorPalette::White();
	const Vector4 numberColor = ColorPalette::Red();

	// テキストの生成&初期化
	auto setup = [&](Element& e, Font* font, const std::wstring& str, Vector2 pos, Vector2 inOff, Vector2 outOff, Vector2 drift, float delay, float startScale, Vector4 color) {
		e.text = std::make_unique<TextSprite>();
		e.text->Initialize(ctx, font, str);
		e.text->SetAnchorPoint({0.5f, 0.5f});
		e.basePos = pos;
		e.inOffset = inOff;
		e.outOffset = outOff;
		e.driftPerSec = drift;
		e.delay = delay;
		e.startScale = startScale;
		e.color = color;
	};

	// Stage左から
	setup(elements_[0], font_.get(), L"Stage", {400.0f, 230.0f}, {-1000.0f, 0.0f}, {1500.0f, 0.0f}, {12.0f, 0.0f}, 0.0f, 1.0f, textColor);

	// Clear右から
	setup(elements_[1], font_.get(), L"Clear", {560.0f, 500.0f}, {1000.0f, 0.0f}, {-1500.0f, 0.0f}, {-12.0f, 0.0f}, 0.08f, 1.0f, textColor);

	// 数字上から
	setup(elements_[2], numberFont_.get(), std::to_wstring(stageNo), {1030.0f, 360.0f}, {0.0f, -600.0f}, {0.0f, 700.0f}, {0.0f, 0.0f}, 0.16f, 1.6f, numberColor);

	// 背面の帯
	const float kBandDelay[2] = {0.0f, 0.06f};
	const float kBandHeight[2] = {400.0f, 320.0f};
	const Vector4 kBandColor[2] = {
	    ColorPalette::PastelRed(), // 奥
	    ColorPalette::Black()      // 手前
	};
	for (int i = 0; i < 2; ++i) {
		Band& b = bands_[i];
		b.sprite = std::make_unique<Sprite>();
		b.sprite->Initialize(ctx, "white.png");
		b.sprite->SetAnchorPoint({0.5f, 0.5f});
		b.sprite->SetPosition({640.0f, 360.0f});
		b.sprite->SetSize({1280.0f, 0.0f});
		b.sprite->SetRotation(rotation_);
		b.delay = kBandDelay[i];
		b.height = kBandHeight[i];
		b.color = kBandColor[i];
		b.sprite->SetColor(b.color);
	}

	// 最後の要素の退場が終わるまで
	totalDuration_ = kInDuration_ + kHoldDuration_ + kOutDuration_ + elements_.back().delay;
}

void StageClearCutIn::Trigger() {
	if (isTriggered_) {
		return;
	}
	isTriggered_ = true;
	time_ = 0.0f;
}

void StageClearCutIn::Update(float deltaTime) {
	if (!isTriggered_ || IsFinished()) {
		return;
	}
	time_ += deltaTime;

	const float outStart = kInDuration_ + kHoldDuration_;

	// 帯が広がって最後に閉じる
	for (auto& b : bands_) {
		float tIn = Ease(EaseType::EASEOUTCUBIC, Progress(time_, b.delay, 0.3f));
		float tOut = Ease(EaseType::EASEINCUBIC, Progress(time_, outStart + 0.1f + b.delay, 0.3f));
		b.sprite->SetSize({1400.0f, b.height * tIn * (1.0f - tOut)});
		b.sprite->Update();
	}

	// テキスト
	for (auto& e : elements_) {
		float tIn = Progress(time_, e.delay, kInDuration_);
		float tOut = Progress(time_, outStart + e.delay, kOutDuration_);
		float easedIn = Ease(EaseType::EASEOUTEXPO, tIn);
		float easedOut = Ease(EaseType::EASEINCUBIC, tOut);

		Vector2 pos = {
		    e.basePos.x + e.inOffset.x * (1.0f - easedIn) + e.outOffset.x * easedOut + e.driftPerSec.x * time_,
		    e.basePos.y + e.inOffset.y * (1.0f - easedIn) + e.outOffset.y * easedOut + e.driftPerSec.y * time_,
		};

		e.text->SetPosition(pos);
		e.text->SetRotation(rotation_);
		e.text->SetScale(MathUtility::Lerp(e.startScale, 1.0f, easedIn));
		e.text->SetColor({e.color.x, e.color.y, e.color.z, tIn});
		e.text->Update();
	}
}

void StageClearCutIn::Draw() {
	if (!isTriggered_ || IsFinished()) {
		return;
	}
	for (auto& b : bands_) {
		b.sprite->Draw();
	}
	for (auto& e : elements_) {
		e.text->Draw();
	}
}