#include "Player.h"

using namespace TinyEngine;

void Player::Initialize(EngineContext* ctx) {
	model_ = std::make_unique<Object3d>();
	model_->Initialize(ctx);

	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {0.0f, 0.0f, 0.0f};
}

void Player::Update() {
	model_->SetTransform(transform_);
	model_->Update();
}

void Player::Draw() { model_->Draw(); }