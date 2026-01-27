#pragma once
#include "AnimationBundle.h"
#include "EngineContext.h"
#include "ScreenSpaceUtility.h"
#include "Sprite.h"
#include <memory>

class CrossEffect {
public:
	void Initialize(EngineContext* ctx, Vector3 targetPos);

	void Update(float deltaTime, DebugCamera* camera);

	void Draw();

	// Getter
	bool GetIsAnimation() const { return crossScaleAnimation_.anim.GetIsActive(); }

private:
	// スプライト
	std::unique_ptr<Sprite> crossSprite_;

	// アニメーション
	AnimationBundle<Vector2> crossScaleAnimation_;

	// 目標ポジション
	Vector3 targetPos_;

	// 座標変換便利クラス
	std::unique_ptr<ScreenSpaceUtility> screenSpaceUtility_;
};
