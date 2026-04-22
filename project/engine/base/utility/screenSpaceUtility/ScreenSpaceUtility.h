#pragma once
#include "DebugCamera.h"
#include "Vector2.h"
#include "Vector3.h"

/// <summary>
/// スクリーン空間ユーティリティクラス
/// </summary>
class ScreenSpaceUtility {
public:
	Vector2 WorldToScreen(const Vector3& worldPos, const Vector2& margin = {0, 0}) const;

	void SetCamera(Camera* camera) { camera_ = camera; }

private:
	Camera* camera_ = nullptr;
};
