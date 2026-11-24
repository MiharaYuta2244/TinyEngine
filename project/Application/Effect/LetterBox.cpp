#include "LetterBox.h"

using namespace TinyEngine;

void LetterBox::Initialize(EngineContext* ctx) {
	for (auto& sprite : sprites_) {
		sprite = std::make_unique<Sprite>();
		sprite->Initialize(ctx, "white.png");
		sprite->SetSize({screenWidth_, boxHeight_});
		sprite->SetColor({0.0f, 0.0f, 0.0f, 1.0f});
	}

	easing_ = EasingAnimation<float>(0.0f, 1.0f, 1.0f, EaseType::EASEOUTCUBIC);
}

void LetterBox::Update(float deltaTime) {
	if (!isTriggered_ || isHidden_)
		return;

	if (isHiding_) {
		// 退場中
		hideEasing_.Update(deltaTime, progress_);
		if (!hideEasing_.GetIsActive()) {
			isHiding_ = false;
			isHidden_ = true;
		}
	} else {
		easing_.Update(deltaTime, progress_);
	}

	float topStartY = -boxHeight_;
	float topEndY = 0.0f;
	positionTop_ = {0.0f, std::lerp(topStartY, topEndY, progress_)};

	float bottomStartY = screenHeight_;
	float bottomEndY = screenHeight_ - boxHeight_;
	positionBottom_ = {0.0f, std::lerp(bottomStartY, bottomEndY, progress_)};

	// 座標の適用
	sprites_[0]->SetPosition(positionTop_);
	sprites_[1]->SetPosition(positionBottom_);

	for (auto& sprite : sprites_) {
		sprite->Update();
	}
}

void LetterBox::Draw() {
	if (!isTriggered_ || isHidden_)
		return;

	for (auto& sprite : sprites_) {
		sprite->Draw();
	}
}

void LetterBox::Trigger() {
	if (!isTriggered_) {
		isTriggered_ = true;
		easing_.Start(0.0f, 1.0f, 1.5f, EaseType::EASEOUTQUART);
	}
}

void LetterBox::Hide() {
	if (!isTriggered_ || isHiding_ || isHidden_)
		return;

	// 登場アニメーション途中でも、現在位置から退場できるようにする
	easing_.Reset();
	isHiding_ = true;
	hideEasing_.Start(progress_, 0.0f, 0.6f, EaseType::EASEINCUBIC);
}