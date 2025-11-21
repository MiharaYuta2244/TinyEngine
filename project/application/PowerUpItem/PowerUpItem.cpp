#include "PowerUpItem.h"

void PowerUpItem::Initialize(EngineContext* ctx) {
	// コンテキスト構造体
	ctx_ = ctx;

	// 3Dオブジェクトの生成
	object3d_ = std::make_unique<Object3d>();

	// Object3dの初期化
	object3d_->Initialize(ctx_);
	object3d_->SetColor({1.0f, 0.0f, 0.0f, 1.0f});

	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {20.0f, 3.0f, 0.0f};
	size_ = {1.0f, 1.0f, 1.0f};
	collisionSize = {1.0f, 1.0f, 1.0f};
	isActive_ = true;

	// 当たり判定の位置更新
	UpdateCollisionPos();
}

void PowerUpItem::Update(float deltaTime) {
	// 当たり判定更新
	UpdateCollisionPos();

	// 位置の更新
	object3d_->SetTransform(transform_);

	// Object3dの更新
	object3d_->Update();
}

void PowerUpItem::Draw() { object3d_->Draw(); }

void PowerUpItem::UpdateCollisionPos() {
	// 当たり判定の更新
	aabb_.min = {transform_.translate.x - 0.5f, transform_.translate.y - 0.5f, transform_.translate.z - 0.5f};
	aabb_.max = {transform_.translate.x + 0.5f, transform_.translate.y + 0.5f, transform_.translate.z + 0.5f};
	sphere_.center = transform_.translate;
	sphere_.radius = 0.5f;
}