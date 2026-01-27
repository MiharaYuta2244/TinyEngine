#include "IFruit.h"

void IFruit::Initialize(EngineContext* ctx, const std::string& fruitName) {
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(ctx);
	object3d_->SetModel(fruitName + ".obj");
	object3d_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	object3d_->GetMaterial().enableLighting = false;

	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, std::numbers::pi_v<float>, 0.0f};

	// 名前を設定
	fruitName_ = "fruitName";

	// 回転アニメーション初期設定
	SettingRotateAnimation();
}

void IFruit::Update(float deltaTime) {
	// 回転アニメーション
	RotateAnimation(deltaTime);

	// Transformの更新
	object3d_->SetTransform(transform_);

	// 当たり判定の更新
	aabb_.min = {transform_.translate.x - 0.5f, transform_.translate.y - 0.5f, transform_.translate.z - 0.5f};
	aabb_.max = {transform_.translate.x + 0.5f, transform_.translate.y + 0.5f, transform_.translate.z + 0.5f};

	object3d_->Update();
}

void IFruit::Draw() { object3d_->Draw(); }

void IFruit::RotateAnimation(float deltaTime) {
	// 回転アニメーション更新
	bool playing = rotateAnimation_.anim.Update(deltaTime, rotateAnimation_.temp);
	transform_.rotate.y = rotateAnimation_.temp;

	if (!playing) {
		// 回転アニメーション初期設定
		SettingRotateAnimation();
	}
}

void IFruit::SettingRotateAnimation() { rotateAnimation_.anim = {0.0f, std::numbers::pi_v<float> * 3.0f, 1.0f, EaseType::EASEINOUTSINE}; }
