#include "CameraDeathZoomController.h"
#include "Easing.h"
#include "MathUtility.h"
#include <algorithm>
#include <numbers>

void CameraDeathZoomController::Start(const Vector3& startPos, const Vector3& startEuler, const Vector3& targetPos) {
	if (isActive_)
		return;

	// Y演出
	currentY_ = startPos.y;
	step_ = 0;
	changeTimer_.Initialize(changeDuration_);

	// XZ移動・回転演出
	startPos_ = startPos;
	targetPos_ = targetPos;
	startEuler_ = startEuler;
	targetEuler_ = {std::numbers::pi_v<float> / 2.0f, startEuler.y, 0.0f};
	currentEuler_ = startEuler_;
	totalElapsed_ = 0.0f;

	isActive_ = true;
	isFinished_ = false;
}

Vector3 CameraDeathZoomController::Update(float deltaTime) {
	// XZ座標・回転の補間
	totalElapsed_ += deltaTime;
	float totalDuration = static_cast<float>(maxStep_) * changeDuration_;
	float rotT = 0.0f;
	if (totalDuration > 0.0f) {
		rotT = std::clamp(totalElapsed_ / totalDuration, 0.0f, 1.0f);
	}
	float easedRotT = Easing::ApplyEasing(EaseType::EASEOUTCUBIC, rotT);

	currentEuler_.x = MathUtility::LerpAngle(startEuler_.x, targetEuler_.x, easedRotT);
	currentEuler_.y = startEuler_.y;
	currentEuler_.z = 0.0f;

	// Y座標
	if (step_ >= maxStep_) {
		isFinished_ = true;
	} else {
		changeTimer_.Update(deltaTime);
		if (changeTimer_.IsEnd()) {
			currentY_ += addAmount_;
			step_++;
			changeTimer_.Initialize(changeDuration_);
		}
	}

	// XZ座標
	float x = startPos_.x;
	float z = startPos_.z;
	if (step_ >= maxStep_) {
		x = targetPos_.x;
		z = targetPos_.z;
	}

	return {x, currentY_, z};
}

void CameraDeathZoomController::Skip() {
	step_ = maxStep_;
	totalElapsed_ = static_cast<float>(maxStep_) * changeDuration_;
}