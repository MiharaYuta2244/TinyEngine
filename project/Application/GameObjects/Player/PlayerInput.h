#pragma once
#include "DirectInput.h"
#include "EngineContext.h"
#include "GamePad.h"

struct PlayerInputState {
	Vector2 moveDir{};          // 移動入力
	Vector2 aimDir{};           // 向き入力
	bool grabHeld = false;      // 掴み押しっぱなし
	bool grabPressed = false;   // 掴み押した瞬間
	bool grabReleased = false;  // 掴み離した瞬間
	bool attackPressed = false; // 攻撃を押した瞬間
};

/// <summary>
/// プレイヤーの入力クラス
/// </summary>
class PlayerInput {
public:
	// 入力の結果を返す関数
	PlayerInputState Poll(DirectInput* input, GamePad* gamePad, EngineContext* ctx, const Vector3& playerPos, bool isHold);

private:
	// マウスのレイ計算
	Vector2 PollMouseAim(DirectInput* input, EngineContext* ctx, const Vector3& playerPos) const;

private:
	float preLt_ = 0.0f;
	float preRt_ = 0.0f;
};