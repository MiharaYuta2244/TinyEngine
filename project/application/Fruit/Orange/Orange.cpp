#include "Orange.h"
#include <numbers>

void Orange::Initialize(EngineContext* ctx) {
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(ctx);
	object3d_->SetModel("orange.obj");
	object3d_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, std::numbers::pi_v<float>, 0.0f};

	// 名前を設定
	fruitName_ = "Orange";
}

void Orange::Update(float deltaTime) {
	(void)deltaTime;

	// 位置の更新
	object3d_->SetTransform(transform_);

	// 当たり判定の更新
	aabb_.min = {transform_.translate.x - 0.5f, transform_.translate.y - 0.5f, transform_.translate.z - 0.5f};
	aabb_.max = {transform_.translate.x + 0.5f, transform_.translate.y + 0.5f, transform_.translate.z + 0.5f};

	object3d_->Update();
}

void Orange::Draw() { object3d_->Draw(); }