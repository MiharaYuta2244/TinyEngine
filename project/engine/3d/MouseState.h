#pragma once
#include <Windows.h>

/// <summary>
/// マウスの状態を表す構造体
/// </summary>
struct MouseState
{
	float x;
	float y;
	float z;
	BYTE rgbButtons[8];
};