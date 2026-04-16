#include "FadeManager.h"
#include "FadeStates.h"

using namespace TinyEngine;

void FadeManager::Initialize(EngineContext* ctx) {
	fadeSprite_ = std::make_unique<Sprite>();
	fadeSprite_->Initialize(ctx, "white.png");
	fadeSprite_->SetPosition({640.0f, 360.0f});
	fadeSprite_->SetSize({1280.0f, 720.0f});
	fadeSprite_->SetAnchorPoint({0.5f, 0.5f});
	fadeSprite_->SetColor({0.0f, 0.0f, 0.0f, 0.0f});

	ChangeState(std::make_unique<FadeStateNone>());
}

void FadeManager::Update(float deltaTime) {
	if (currentState_) {
		currentState_->Update(this, deltaTime);
	}
}

void FadeManager::Draw() {
	if (fadeSprite_) {
		fadeSprite_->Update();
		fadeSprite_->Draw();
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

void FadeManager::SetFadeAlpha(float alpha) { fadeSprite_->SetColor({0.0f, 0.0f, 0.0f, alpha}); }
