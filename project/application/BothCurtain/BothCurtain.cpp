#include "BothCurtain.h"

void BothCurtain::Initialize(EngineContext* ctx) {
	spriteRightCurtain_ = std::make_unique<Sprite>();
	spriteRightCurtain_->Initialize(ctx, "resources/white.png");
	spriteRightCurtain_->SetPosition({0.0f, 0.0f});
	spriteRightCurtain_->SetSize({160.0f, 720.0f});
	spriteRightCurtain_->SetColor({0.0f,0.0f,0.0f,1.0f});

	spriteLeftCurtain_ = std::make_unique<Sprite>();
	spriteLeftCurtain_->Initialize(ctx, "resources/white.png");
	spriteLeftCurtain_->SetPosition({1120.0f, 0.0f});
	spriteLeftCurtain_->SetSize({160.0f, 720.0f});
	spriteLeftCurtain_->SetColor({0.0f, 0.0f, 0.0f, 1.0f});
}

void BothCurtain::Update() {
	spriteRightCurtain_->Update();
	spriteLeftCurtain_->Update();
}

void BothCurtain::Draw() {
	spriteRightCurtain_->Draw();
	spriteLeftCurtain_->Draw();
}