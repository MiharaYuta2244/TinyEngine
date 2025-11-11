#include "Enemy.h"
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "TextureManager.h"

void Enemy::Initialize(Object3dCommon* obj3dCommon, TextureManager* texMane, ModelManager* ModelMane) {
	// 3Dオブジェクトの生成
	object3d_ = std::make_unique<Object3d>();

	// Object3dの初期化
	object3d_->Initialize(obj3dCommon, texMane, ModelMane);
	object3d_->SetColor({1.0f, 0.0f, 0.0f, 1.0f});

	transform_.scale = {2.0f, 2.0f, 2.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {20.0f, 2.0f, 0.0f};
	size_ = {1.0f, 1.0f, 1.0f};
	collisionSize = {1.0f, 1.0f, 1.0f};
	isActive_ = true;

	// 当たり判定の位置更新
	UpdateCollisionPos();
}

void Enemy::Update(float deltaTime) {
	// 経過時間
	deltaTime_ = deltaTime;

	// 左右移動
	HorizontalMove();

	// 反転処理
	DirectionChange();

	// 当たり判定の位置更新
	UpdateCollisionPos();

	// 位置の更新
	object3d_->SetTransform(transform_);

	// Object3dの更新
	object3d_->Update();
}

void Enemy::Draw() { object3d_->Draw(); }

void Enemy::UpdateImGui() {
	ImGui::Begin("Enemy");

	ImGui::DragFloat3("Position", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat2("Velocity", &velocity_.x, 0.01f);

	ImGui::End();
}

void Enemy::HorizontalMove() {
	if (direction_ == Direction::RIGHT) {
		transform_.translate.x += velocity_.x * deltaTime_;
	} else if (direction_ == Direction::LEFT) {
		transform_.translate.x -= velocity_.x * deltaTime_;
	}
}

void Enemy::DirectionChange() {
	if (transform_.translate.x >= 10.0f) {
		direction_ = Direction::LEFT;
	} else if (transform_.translate.x <= -10.0f) {
		direction_ = Direction::RIGHT;
	}
}

void Enemy::UpdateCollisionPos() {
	aabb_.min = {transform_.translate.x - 0.5f, transform_.translate.y - 0.5f, transform_.translate.z - 0.5f};
	aabb_.max = {transform_.translate.x + 0.5f, transform_.translate.y + 0.5f, transform_.translate.z + 0.5f};
	sphere_.center = transform_.translate;
	sphere_.radius = 0.5f;
}