#include "Apple.h"
#include <numbers>

void Apple::Initialize(EngineContext* ctx) {
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(ctx);
	object3d_->SetModel("apple.obj");
	object3d_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, std::numbers::pi_v<float> + 0.05f, 0.0f};

	// 名前を設定
	fruitName_ = "Apple";
}

void Apple::Update(float deltaTime) {
	(void)deltaTime;

	// 位置の更新
	object3d_->SetTransform(transform_);

	object3d_->Update();
}

void Apple::Draw() { object3d_->Draw(); }