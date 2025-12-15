#pragma once
#include <memory>
#include "Sprite.h"
#include "EngineContext.h"

class EnemyHPGauge {
public:
	void Initialize(EngineContext* ctx);

	void Update();

	void Draw();

	// HP量をスプライトのサイズに適用
	void HPBarSpriteApply(int currentHP, int maxHP);

private:
	// 仮のHPバー
	std::unique_ptr<Sprite> spriteHPBar_ = std::make_unique<Sprite>();
	std::unique_ptr<Sprite> spriteHPBarBG_ = std::make_unique<Sprite>();

	// HPバーのサイズX
	const float kMaxSpriteHPBarSizeX = 600.0f;
	float spriteHPBarSizeX_ = kMaxSpriteHPBarSizeX;

	// コンテキスト構造体
	EngineContext* ctx_;
};
