#pragma once
#include "AnimationBundle.h"
#include "IHPGauge.h"
#include "Sprite.h"
#include <memory>

class PlayerHPGauge : public IHPGauge {
public:
	void Initialize(EngineContext* ctx) override;

	void Update(float deltaTime) override;

	void Draw() override;

	// HP量をスプライトのサイズに適用
	void HPBarSpriteApply(int currentHP, int maxHP) override;

	// setter
	void SetPosition(const Vector2& position) {
		spriteHPBar_->SetPosition(position);
		spriteHPBarAfter_->SetPosition(position);
		spriteHPBarBG_->SetPosition({position.x - 5.0f, position.y - 5.0f});
	}

	// ImGui
	void DrawImGui();

private:
	// HPの残像アニメーション
	void AnimationHPGauge(float deltaTime) override;

private:
	// 座標
	Vector2 position_ = {40.0f, 40.0f};
};