#pragma once
#include "Vector4.h"

/// <summary>
/// GPUに送るアウトラインの構造体
/// </summary>
struct Outline{
	Vector4 color;
	float thickness;
};