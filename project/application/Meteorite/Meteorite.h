#pragma once
#include "AnimationBundle.h"
#include "EngineContext.h"
#include "Object3d.h"
#include <memory>

class Meteorite {
public:
	void Initialize(EngineContext* ctx);

	void Update(float dt);

	void Draw();

	// 座標のSetter
	void SetTranslate(Vector3 translate) { meteoriteModel_->SetTranslate(translate); }

	// アニメーション終了判定Getter
	bool GetEndAnimation() const { return fallAnimation_.anim.GetIsActive(); }

private:
	// 落下
	void Fall();

	// アニメーションの初期設定
	void SettingFallAnimation();

private:
	// モデル
	std::unique_ptr<Object3d> meteoriteModel_;

	// 落下アニメーション
	AnimationBundle<float> fallAnimation_;

	// 経過時間
	float dt_ = 0.0f;
};
