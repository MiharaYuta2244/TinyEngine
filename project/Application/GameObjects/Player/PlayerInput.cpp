#include "PlayerInput.h"
#include "MathOperator.h"

PlayerInputState PlayerInput::Poll(DirectInput* input, GamePad* gamePad, EngineContext* ctx, const Vector3& playerPos, bool isHold) {
	PlayerInputState s;

	if (input->KeyDown(DIK_D))
		s.moveDir.x += 1.0f;
	if (input->KeyDown(DIK_A))
		s.moveDir.x -= 1.0f;
	if (input->KeyDown(DIK_W))
		s.moveDir.y += 1.0f;
	if (input->KeyDown(DIK_S))
		s.moveDir.y -= 1.0f;

	const bool padConnected = gamePad && gamePad->GetState().connected;
	if (!padConnected) {
		s.aimDir = PollMouseAim(input, ctx, playerPos);
	}

	s.grabHeld = input->MouseButtonDown(1);
	s.grabReleased = input->MouseButtonReleased(1);
	s.grabPressed = input->MouseButtonTriggered(1);
	s.attackPressed = input->MouseButtonTriggered(0);

	if (padConnected) {
		const auto& pad = gamePad->GetState();
		const auto& padState = gamePad->GetState();

		// 十字キー
		if (padState.buttons.dpadRight)
			s.moveDir.x += 1.0f;
		if (padState.buttons.dpadLeft)
			s.moveDir.x -= 1.0f;
		if (padState.buttons.dpadUp)
			s.moveDir.y += 1.0f;
		if (padState.buttons.dpadDown)
			s.moveDir.y -= 1.0f;

		// 左スティック入力（デッドゾーンを超えていれば上書き）
		if (std::abs(padState.axes.lx) > 0.1f || std::abs(padState.axes.ly) > 0.1f) {
			s.moveDir.x = padState.axes.lx;
			s.moveDir.y = -padState.axes.ly;
		}

		// 右スティック入力
		if (std::abs(padState.axes.rx) > 0.1f || std::abs(padState.axes.ry) > 0.1f) {
			s.aimDir.x = padState.axes.rx;
			s.aimDir.y = -padState.axes.ry;
		}

		preLt_ = pad.axes.lt;
		preRt_ = pad.axes.rt;
	} else {
		preLt_ = preRt_ = 0.0f;
	}

	// 斜め入力のクランプ
	float len = MathUtility::Length(s.moveDir);
	if (len > 1.0f) {
		s.moveDir.x /= len;
		s.moveDir.y /= len;
	}

	return s;
}

Vector2 PlayerInput::PollMouseAim(DirectInput* input, EngineContext* ctx, const Vector3& playerPos)const{
	Vector2 aimDir = {0};

	// マウスカーソル位置からプレイヤーの向く方向を計算
	if (ctx && ctx->object3dCommon) {
		Camera* camera = ctx->object3dCommon->GetDefaultCamera();
		if (camera) {
			Vector2 mousePos = input->GetMousePosition();

			// スクリーン座標からNDC座標
			float nx = (2.0f * mousePos.x) / static_cast<float>(WinApp::kClientWidth) - 1.0f;
			float ny = 1.0f - (2.0f * mousePos.y) / static_cast<float>(WinApp::kClientHeight);

			Matrix4x4 viewMat = camera->GetViewMatrix();
			Matrix4x4 projMat = camera->GetProjection();
			Matrix4x4 vpInv = MathUtility::Inverse(MathUtility::Multiply(viewMat, projMat));

			Vector3 nearPos = {nx, ny, 0.0f};
			Vector3 farPos = {nx, ny, 1.0f};
			Vector3 nearWorld = MathUtility::Transform(nearPos, vpInv);
			Vector3 farWorld = MathUtility::Transform(farPos, vpInv);
			Vector3 rayDir = MathUtility::Normalize(farWorld - nearWorld);

			// プレイヤーの足元の高さの平面とレイの交点を求める
			float planeY = playerPos.y;
			if (std::abs(rayDir.y) > 0.0001f) {
				float t = (planeY - nearWorld.y) / rayDir.y;
				if (t > 0.0f) {
					Vector3 hitPos = nearWorld + rayDir * t;
					Vector3 toCursor = hitPos - playerPos;
					toCursor.y = 0.0f;

					if (MathUtility::LengthSquared(toCursor) > 0.0001f) {
						aimDir = {toCursor.x, toCursor.z};
					}
				}
			}
		}
	}

	return aimDir;
}