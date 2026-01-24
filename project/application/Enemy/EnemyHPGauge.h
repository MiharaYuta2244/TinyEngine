#pragma once
#include <memory>
#include "Sprite.h"
#include "EngineContext.h"
#include "AnimationBundle.h"

class EnemyHPGauge {
public:
	void Initialize(EngineContext* ctx);

	void Update(float deltaTime);

	void Draw();

	// HP量をスプライトのサイズに適用
	void HPBarSpriteApply(int currentHP, int maxHP);

private:
	// HPバースプライト
	std::unique_ptr<Sprite> spriteHPBar_;
	std::unique_ptr<Sprite> spriteHPBarAfter_;
	std::unique_ptr<Sprite> spriteHPBarBG_;

	// HPバーのサイズX
	const float kMaxSpriteHPBarSizeX = 600.0f;
	float spriteHPBarSizeX_ = kMaxSpriteHPBarSizeX;
	float spriteHPBarAfterSizeX_ = kMaxSpriteHPBarSizeX;

	// 残像の更新遅延フレーム数
	const float kAfterImageDelayFrames = 0.5f;
	float afterImageDelayCounter_ = 0;

	// コンテキスト構造体
	EngineContext* ctx_;

	// HP残像アニメーション
	AnimationBundle<float> afterAnimation_;
};
