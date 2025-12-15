#pragma once
#include "Sprite.h"
#include "EngineContext.h"
#include <memory>

class BothCurtain {
public:
	void Initialize(EngineContext* ctx);
	void Update();
	void Draw();

private:
	std::unique_ptr<Sprite> spriteRightCurtain_; // 右端黒幕
	std::unique_ptr<Sprite> spriteLeftCurtain_;  // 左端黒幕
};
