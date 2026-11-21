#include "Barricade.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace {
constexpr float kPullStageTime = 0.4f;
constexpr int kPullStages = 3;
constexpr float kPullTravel = 0.5f;
constexpr float kGripHeight = 1.44f;
constexpr float kGripDepth = 2.0f;
constexpr float kThrowSpeed = 30.0f;
constexpr float kThrowRange = 24.0f;

Vector3 GetGripOffset(const Transform& pose, float side) {
	auto matrix = MathUtility::MakeAffineMatrix(pose.scale, pose.rotate, Vector3{});
	Vector3 grip = {0.0f, kGripHeight, side * kGripDepth};
	return MathUtility::Transform(grip, matrix);
}

Vector3 GetYawHalfExtent(const Transform& pose) {
	const float c = std::abs(std::cos(pose.rotate.y));
	const float s = std::abs(std::sin(pose.rotate.y));
	const float x = std::abs(pose.scale.x);
	const float z = std::abs(pose.scale.z);
	return {c * x + s * z, std::abs(pose.scale.y), s * x + c * z};
}
}

int Barricade::index = 0;

Barricade::Barricade() { id_ = index++; }

void Barricade::Initialize(EngineContext* ctx, const Transform& transform) {
	transform_ = transform;
	UpdateCollision();

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Barricade.obj");
	render_->SetTransform(transform_);
	render_->SetEnableLighting(true);
}

void Barricade::Update(float deltaTime) {
	if (IsSolid()) {
		UpdateCollision();
		render_->Update(IsPulling() ? GetPullPose() : transform_);
	} else if (IsThrown()) {
		const float step = std::min(kThrowSpeed * deltaTime, throwDistanceLeft_);
		throwStep_ = {shieldPose_.translate, {throwDirection_.x * step, 0.0f, throwDirection_.y * step}};
		shieldPose_.translate = throwStep_.origin + throwStep_.diff;
		throwDistanceLeft_ -= step;
		render_->Update(shieldPose_);
	} else if (state_ == State::Dropped) {
		// 落下位置を保ったまま、現在のカメラで描画行列を更新する
		render_->Update(shieldPose_);
	}

	// ギズモ用当たり判定更新
	UpdateAABBForGizmo();
}

void Barricade::UpdateCollision() {
	const Transform pose = IsPulling() ? GetPullPose() : transform_;
	const Vector3 pos = pose.translate;
	const Vector3 half = GetYawHalfExtent(pose);
	collision_.max = {pos.x + half.x, pos.y, pos.z + half.z};
	collision_.min = {pos.x - half.x, pos.y, pos.z - half.z};
}

Vector2 Barricade::GetGrabOffset(const Vector3& playerPos) const {
	return {
	    playerPos.x - std::clamp(playerPos.x, collision_.min.x, collision_.max.x),
	    playerPos.z - std::clamp(playerPos.z, collision_.min.z, collision_.max.z)};
}

void Barricade::Grab(const Vector3& playerPos) {
	const Vector2 normal = {std::sin(transform_.rotate.y), std::cos(transform_.rotate.y)};
	const Vector3 offset = playerPos - transform_.translate;
	gripSide_ = offset.x * normal.x + offset.z * normal.y < 0.0f ? -1.0f : 1.0f;
	pullDirection_ = {normal.x * gripSide_, normal.y * gripSide_};
	pullTime_ = 0.0f;
	state_ = State::Pulling;
}

Transform Barricade::GetPullPose() const {
	Transform pose = transform_;
	const float travel = kPullTravel * std::clamp(pullTime_ / (kPullStageTime * kPullStages), 0.0f, 1.0f);
	pose.translate.x += pullDirection_.x * travel;
	pose.translate.z += pullDirection_.y * travel;
	return pose;
}

Vector3 Barricade::GetPullGripPosition() const {
	const Transform pose = GetPullPose();
	return pose.translate + GetGripOffset(pose, gripSide_);
}

void Barricade::Pull(const Vector2& moveDir, float deltaTime) {
	const float outwardInput = moveDir.x * pullDirection_.x + moveDir.y * pullDirection_.y;
	if (outwardInput <= 0.1f) {
		pullTime_ = 0.0f;
		return;
	}

	pullTime_ += deltaTime;
	if (pullTime_ >= kPullStageTime * kPullStages) {
		state_ = State::Shield;
	}
}

void Barricade::PlaceShield(const Vector3& handPosition, float yaw) {
	shieldPose_.rotate = {0.0f, yaw, 0.0f};
	shieldPose_.translate = handPosition - GetGripOffset(shieldPose_, -1.0f);
	render_->Update(shieldPose_);
}

void Barricade::Release() {
	if (IsShield()) {
		DropShield();
	} else {
		state_ = State::Attached;
		pullTime_ = 0.0f;
	}
}

void Barricade::Throw(const Vector3& playerPos, const Vector2& forward, float yaw) {
	// 手元からの経路も判定し、至近距離の敵や壁を飛び越えないようにする
	shieldPose_.translate = {playerPos.x, playerPos.y + 1.0f, playerPos.z};
	shieldPose_.rotate = {0.0f, yaw, 0.0f};
	throwDirection_ = forward;
	throwDistanceLeft_ = kThrowRange;
	state_ = State::Thrown;
}

bool Barricade::CheckThrowHit(const AABB& target, float& hitTime) const {
	// 盾の半幅だけ対象を広げ、フレーム間の移動区間で衝突を調べる
	const Vector3 half = GetYawHalfExtent(shieldPose_);
	const AABB expanded{target.min - half, target.max + half};
	return Collision::Intersect(throwStep_, expanded, hitTime);
}

void Barricade::LandThrow(float hitTime) {
	shieldPose_.translate = throwStep_.origin + throwStep_.diff * hitTime;
	DropShield();
}

void Barricade::DropShield() {
	state_ = State::Dropped;
	shieldPose_.translate.y -= 0.8f;
	shieldPose_.rotate.x = std::numbers::pi_v<float> / 2.0f;
	render_->Update(shieldPose_);
}

void Barricade::Draw() {
	outline_.Apply(*render_, CanGrab() || IsPulling());

	// 描画
	render_->Draw();
}
