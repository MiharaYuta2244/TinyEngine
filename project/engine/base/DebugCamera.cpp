#include "DebugCamera.h"
#include "MathUtility.h"
#include "WinApp.h"
#include "MathOperator.h"
#include <numbers>

DebugCamera::DebugCamera()
    : transform_({
          {1.0f, 1.0f, 1.0f  },
          {0.0f, 0.0f, 0.0f  },
          {0.0f, 0.0f, -50.0f}
}),
      euler_({0.0f, 0.0f, 0.0f}), fovY_(0.45f), aspectRatio_(float(WinApp::kClientWidth) / float(WinApp::kClientHeight)), nearClip_(0.1f), farClip_(1000.0f),
      orientation_(MathUtility::MakeIdentity4x4()), pivot_({0.0f, 0.0f, 0.0f}) {
	UpdateViewMatrix();
}

void DebugCamera::Initialize() { UpdateViewMatrix(); }

void DebugCamera::SetPivot(const Vector3& p) {
	Vector3 offset = MathUtility::Subtract(transform_.translate, pivot_);
	pivot_ = p;
	transform_.translate = MathUtility::Add(pivot_, offset);
}

void DebugCamera::SetRotate(const Vector3& rotate) {
	euler_ = rotate;
	euler_.z = 0.0f; // roll は常に禁止
	UpdateOrientation();
	UpdateViewMatrix();
}

void DebugCamera::UpdateOrientation() {
	Matrix4x4 pitch = MathUtility::MakePitchRotateMatrix(euler_.x);
	Matrix4x4 yaw = MathUtility::MakeYawRotateMatrix(euler_.y);

	// roll は使わない
	orientation_ = MathUtility::Multiply(pitch, yaw);
}

void DebugCamera::Update(const DirectInput& input, const GamePad& gamePad) {

	// ============================
	//  マウス右ドラッグ：自由回転
	// ============================
	if (input.MouseButtonDown(1)) {
		float dx = input.GetMouseDeltaX() * 0.002f;
		float dy = input.GetMouseDeltaY() * 0.002f;

		euler_.y += dx;  // yaw
		euler_.x += dy;  // pitch
		euler_.z = 0.0f; // roll 禁止

		UpdateOrientation();
	}

	// ============================
	//  ホイールドラッグ：原点中心回転
	// ============================
	if (!input.KeyDown(DIK_LSHIFT) && input.MouseButtonDown(2)) {
		float dx = input.GetMouseDeltaX() * 0.002f;
		float dy = input.GetMouseDeltaY() * 0.002f;

		// カメラの向き（pitch, yaw）は更新
		euler_.y += dx;
		euler_.x += dy;
		euler_.z = 0.0f;

		UpdateOrientation();

		// pivot回転はyawのみを使う
		Matrix4x4 yawRot = MathUtility::MakeYawRotateMatrix(dx);

		Vector3 offset = transform_.translate - pivot_;
		offset = MathUtility::MultiplyVector(offset, yawRot);
		transform_.translate = pivot_ + offset;
	}

	// ============================
	//  移動処理
	// ============================
	auto MoveLocal = [&](const Vector3& local) {
		Vector3 world = MathUtility::MultiplyVector(local, orientation_);
		transform_.translate = MathUtility::Add(transform_.translate, world);
		pivot_ = MathUtility::Add(pivot_, world);
	};

	const float speed = 0.8f;

	// ホイール前後移動
	float wheelDelta = input.GetMouseWheel();
	if (wheelDelta != 0.0f) {
		float moveSpeed = wheelDelta > 0.0f ? speed : -speed;
		MoveLocal({0.0f, 0.0f, moveSpeed});
	}

	// Shift + ホイールドラッグ：平行移動
	if (input.KeyDown(DIK_LSHIFT) && input.MouseButtonDown(2)) {
		float dx = input.GetMouseDeltaX() * 0.1f;
		float dy = input.GetMouseDeltaY() * 0.1f;

		Vector3 moveVec = {-dx, dy, 0.0f};
		MoveLocal(moveVec);
	}

	UpdateViewMatrix();
}

void DebugCamera::UpdateViewMatrix() {
	worldMatrix_ = MathUtility::Multiply(orientation_, MathUtility::MakeTranslateMatrix(transform_.translate));

	viewMatrix_ = MathUtility::Inverse(worldMatrix_);
	projectionMatrix_ = MathUtility::MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);

	viewProjectionMatrix_ = MathUtility::Multiply(viewMatrix_, projectionMatrix_);
}
