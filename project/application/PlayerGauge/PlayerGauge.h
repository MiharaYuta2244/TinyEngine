#pragma once
#include <memory>
#include "Sprite.h"
#include "EngineContext.h"

class PlayerGauge {
public:
	void Initialize(EngineContext* ctx);
	void Update();
	void Draw();

private:
	std::unique_ptr<Sprite> spriteGaugeBG_; // ゲージ背景
	std::unique_ptr<Sprite> spriteGaugeFill_; // ゲージ全面
};
