#pragma once
#include "AnimationBundle.h"
#include "EngineContext.h"
#include "Sprite.h"
#include <memory>

class IHPGauge {
public:
	virtual ~IHPGauge() {};

	virtual void Initialize(EngineContext* ctx) = 0;

	virtual void Update(float deltaTime) = 0;

	virtual void Draw() = 0;

	// HP量をスプライトのサイズに適用
	virtual void HPBarSpriteApply(int currentHP, int maxHP) = 0;

private:
	// HPの残像アニメーション
	virtual void AnimationHPGauge(float deltaTime) = 0;

protected:
	// HPバースプライト
	std::unique_ptr<TinyEngine::Sprite> spriteHPBar_;
	std::unique_ptr<TinyEngine::Sprite> spriteHPBarAfter_;
	std::unique_ptr<TinyEngine::Sprite> spriteHPBarBG_;

	// HPバーのサイズX
	float maxSpriteHPBarSizeX_ = {};
	float maxSpriteHPBarSizeY_ = {};
	float spriteHPBarSizeX_ = {};
	float spriteHPBarAfterSizeX_ = {};

	// 残像の更新遅延フレーム数
	float afterImageDelayFrames = {};
	float afterImageDelayCounter_ = {};

	// コンテキスト構造体
	EngineContext* ctx_ = nullptr;

	// HP残像アニメーション
	AnimationBundle<float> afterAnimation_;
};