#include "Soap.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include <algorithm>
#include <cmath>
#include <numbers>

int Soap::index = 0;

Soap::Soap() { id_ = index++; }

void Soap::Initialize(EngineContext* ctx, const Transform& transform) {
	transform_ = transform;

	// 本体描画
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Soup.obj");
	render_->SetTransform(transform_);

	// 泡の描画
	bubbleRender_ = std::make_unique<ObjectRender>();
	bubbleRender_->Initialize(ctx, "plane.obj");
	bubbleRender_->SetEnableLighting(false);
	bubbleRender_->SetColor({0.6f, 0.85f, 1.0f, 0.0f});

	UpdateCollision();
}

void Soap::Update(float deltaTime) {
	// 移動処理
	UpdateMove(deltaTime);

	// 当たり判定更新
	UpdateCollision();
	UpdateAABBForGizmo();

	// 泡エリアの更新
	UpdateBubble(deltaTime);

	// 描画用インスタンス更新
	render_->SetTransform(transform_);
	render_->Update();
}

void Soap::Draw() {
	render_->Draw();

	if (isBubbleActive_) {
		bubbleRender_->Draw();
	}
}

void Soap::Throw(const Vector3& velocity) {
	prevPos_ = transform_.translate;
	velocity_ = velocity;
	isThrown_ = true;
	isBubbleActive_ = false; // 投げ直された場合は前回の泡を消す
}

bool Soap::IsDangerous() const {
	float speedSq = velocity_.x * velocity_.x + velocity_.z * velocity_.z;
	return isThrown_ && speedSq >= kDangerousSpeedSq_;
}

void Soap::SetEnableOutline(bool isEnable) {
	if (render_) {
		render_->SetEnableOutline(isEnable);
	}
}

void Soap::UpdateMove(float deltaTime) {
	prevPos_ = transform_.translate;

	if (!isThrown_) {
		return;
	}

	transform_.translate.x += velocity_.x * deltaTime;
	transform_.translate.z += velocity_.z * deltaTime;

	// 摩擦による減速
	float speed = std::sqrtf(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
	if (speed > 0.0f) {
		float drop = friction_ * deltaTime;
		float multiplier = std::max(0.0f, speed - drop) / speed;
		velocity_.x *= multiplier;
		velocity_.z *= multiplier;
	}

	// 十分減速したら停止扱いにして泡を発生させる
	float speedAfter = std::sqrtf(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
	if (speedAfter <= stopThreshold_) {
		velocity_ = {0.0f, 0.0f, 0.0f};
		isThrown_ = false;

		if (!isBubbleActive_) {
			isBubbleActive_ = true;
			bubbleTimer_.Initialize(bubbleDuration_);
		}
	}
}

void Soap::UpdateCollision() {
	Vector3 pos = transform_.translate;
	collision_.max = {pos.x + halfSize_.x, pos.y, pos.z + halfSize_.z};
	collision_.min = {pos.x - halfSize_.x, pos.y, pos.z - halfSize_.z};
}

void Soap::UpdateBubble(float deltaTime) {
	Vector3 pos = transform_.translate;
	bubbleArea_.max = {pos.x + bubbleHalfSize_.x, pos.y, pos.z + bubbleHalfSize_.z};
	bubbleArea_.min = {pos.x - bubbleHalfSize_.x, pos.y, pos.z - bubbleHalfSize_.z};

	if (!isBubbleActive_) {
		return;
	}

	bubbleTimer_.Update(deltaTime);

	// 残り1秒でフェードアウトさせる
	float remaining = bubbleDuration_ - bubbleTimer_.GetTimer();
	float alpha = std::clamp(remaining / 1.0f, 0.0f, 1.0f) * 0.6f;

	bubbleRender_->SetColor({0.6f, 0.85f, 1.0f, alpha});
	bubbleRender_->SetTransform(
	    Transform{
	        {bubbleHalfSize_.x,                 bubbleHalfSize_.y, bubbleHalfSize_.z},
            {-std::numbers::pi_v<float> / 2.0f, 0.0f,              0.0f             },
            {pos.x,                             0.05f,             pos.z            }
    });
	bubbleRender_->Update();

	if (bubbleTimer_.IsEnd()) {
		isBubbleActive_ = false;
	}
}

void Soap::Grab() {
	velocity_ = {0.0f, 0.0f, 0.0f};
	isThrown_ = false;
	isBubbleActive_ = false; // 持ち上げたら泡は消す
}

void Soap::Stop() {
	transform_.translate = prevPos_;
	velocity_ = {0.0f, 0.0f, 0.0f};
	isThrown_ = false;

	isBubbleActive_ = true;
	bubbleTimer_.Initialize(bubbleDuration_);
}