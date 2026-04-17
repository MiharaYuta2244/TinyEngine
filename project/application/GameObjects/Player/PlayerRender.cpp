#include "PlayerRender.h"

using namespace TinyEngine;
using namespace Player;

void Render::Initialize(EngineContext* ctx, DirectInput* input) {
	model_ = std::make_unique<Object3d>();
	model_->Initialize(ctx);
	model_->SetModel("suzanne.obj");
	model_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, std::numbers::pi_v<float> / 2, 0.0f};
	transform_.translate = {0.0f, 0.0f, 0.0f};

	input_ = input;
}

void Render::Update() {
	model_->SetTransform(transform_);
	model_->Update();

#ifdef USE_IMGUI
	ImGui::Begin("Player");
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
	ImGui::End();
#endif // USE_IMGUI
}

void Render::Draw() { model_->Draw(); }

void Player::Render::Move(float deltaTime) {
	Vector3& rotate = model_->GetTransform().rotate;

	// 移動入力の取得
	Vector2 dir{};

	// 正規化
	float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);

	// 入力があるかどうかの判定
	bool isMoving = (length > 0.0f);
	if (isMoving) {
		dir.x /= length;
		dir.y /= length;

		// 目標の角度を求める
		float targetAngle = std::atan2(-dir.x, -dir.y);

		// 現在の角度と目標の角度の差分
		float diff = targetAngle - rotate.y;

		// 差分を収める
		const float kPi = std::numbers::pi_v<float>;
		while (diff < -kPi)
			diff += 2.0f * kPi;
		while (diff > kPi)
			diff -= 2.0f * kPi;

		// 回転を補間
		rotate.y += diff * rotateSpeed_ * deltaTime;

		// 通常移動の適用
		Transform& t = model_->GetTransform();
		t.translate.x += dir.x * Velocity_.x * deltaTime;
		t.translate.z += dir.y * Velocity_.y * deltaTime;
	}
}
