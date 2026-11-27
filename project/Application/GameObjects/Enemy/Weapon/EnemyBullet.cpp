#include "EnemyBullet.h"
#include "MathOperator.h"
#include <algorithm>
#include <cmath>

void EnemyBullet::Initialize(EngineContext* ctx, const Vector3& muzzle, const Vector3& direction, std::uint64_t shooterId) {
	muzzle_ = muzzle;
	direction_ = direction;
	shooterId_ = shooterId;

	transform_.translate = muzzle_;
	transform_.rotate.x = std::atan2(-direction.y, std::hypot(direction.x, direction.z));
	transform_.rotate.y = std::atan2(direction.x, direction.z);
	transform_.scale = {0.2f, 0.2f, 0.0f};

	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cylinder.obj");
	render_->SetEnableLaser(true);
	render_->SetColor({0.0f, 0.0f, 1.0f, 1.0f});

	scaleXYAnim_.anim.Start({0.2f, 0.2f}, {0.0f, 0.0f}, kLifeTime, EaseType::EASEINCUBIC);
}

Vector3 EnemyBullet::GetTip() const { return muzzle_ + direction_ * length_; }

float EnemyBullet::GetGrowthDistance(float deltaTime) const {
	if (hasHit_) {
		return 0.0f;
	}

	const float growthTime = std::clamp(deltaTime, 0.0f, kLifeTime - deathTimer_);
	return std::min(kGrowthSpeed * growthTime, kMaxLength - length_);
}

void EnemyBullet::Extend(float distance, bool hit) {
	length_ = std::min(length_ + distance, kMaxLength);
	hasHit_ = hit;
}

void EnemyBullet::Update(float deltaTime) {
	deathTimer_ += deltaTime;
	if (scaleXYAnim_.anim.Update(deltaTime, scaleXYAnim_.temp)) {
		transform_.scale.x = scaleXYAnim_.temp.x;
		transform_.scale.y = scaleXYAnim_.temp.y;
	}

	// 描画の始点を銃口に固定
	const float halfLength = length_ * 0.5f;
	transform_.scale.z = halfLength;
	transform_.translate = muzzle_ + direction_ * halfLength;
	render_->SetTime(deathTimer_);
	render_->Update(transform_);
}

void EnemyBullet::Draw() {
	if (length_ > 0.0f) {
		render_->Draw();
	}
}
