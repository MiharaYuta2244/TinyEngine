#include "PlayerInput.h"
#include "InputBindings.h"
#include "MathOperator.h"

PlayerInputState PlayerInput::Poll(DirectInput* input, GamePad* gamePad, EngineContext* ctx, const Vector3& playerPos, bool isHold) {
	PlayerInputState s;

	const auto& bindings = InputBindings::Get();
	s.moveDir.x = static_cast<float>(bindings.Down(GameAction::Right, input, gamePad)) - static_cast<float>(bindings.Down(GameAction::Left, input, gamePad));
	s.moveDir.y = static_cast<float>(bindings.Down(GameAction::Up, input, gamePad)) - static_cast<float>(bindings.Down(GameAction::Down, input, gamePad));
	s.grabHeld = bindings.Down(GameAction::Grab, input, gamePad);
	s.grabPressed = bindings.Pressed(GameAction::Grab, input, gamePad);
	s.grabReleased = previousGrabHeld_ && !s.grabHeld;
	previousGrabHeld_ = s.grabHeld;
	s.attackPressed = bindings.Pressed(GameAction::Attack, input, gamePad);

	const bool padConnected = gamePad && gamePad->GetState().connected;
	if (!padConnected) s.aimDir = PollMouseAim(input, ctx, playerPos);
	if (padConnected) {
		const auto& padState = gamePad->GetState();
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