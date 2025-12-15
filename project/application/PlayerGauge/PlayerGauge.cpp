#include "PlayerGauge.h"

void PlayerGauge::Initialize(EngineContext* ctx) {
	spriteGaugeBG_ = std::make_unique<Sprite>();
	spriteGaugeBG_->Initialize(ctx,"resources/white.png");

	spriteGaugeFill_ = std::make_unique<Sprite>();
	spriteGaugeFill_->Initialize(ctx, "resources/white.png");

	spriteGaugeBG_->SetPosition({1200.0f, 100.0f});
	spriteGaugeFill_->SetPosition({1210.0f, 110.0f});

	spriteGaugeBG_->SetSize({40.0f,520.0f});
	spriteGaugeFill_->SetSize({20.0f,500.0f});

	spriteGaugeBG_->SetColor({0.4f,0.4f,0.4f,1.0f});
	spriteGaugeFill_->SetColor({1.0f, 0.0f, 0.0f, 1.0f});
}

void PlayerGauge::Update() {
	if (spriteGaugeBG_)
		spriteGaugeBG_->Update();

	if (spriteGaugeFill_)
		spriteGaugeFill_->Update();
}

void PlayerGauge::Draw() {
	if (spriteGaugeBG_)
		spriteGaugeBG_->Draw();

	if (spriteGaugeFill_)
		spriteGaugeFill_->Draw();
}