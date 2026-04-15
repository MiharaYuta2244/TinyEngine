#pragma once
#include "Matrix4x4.h"
#include "Vector4.h"
#include <widemath.h>

/// <summary>
/// GPUに送るマテリアルの構造体
/// </summary>
struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
	float shininess;
	int32_t enableFoging;
};