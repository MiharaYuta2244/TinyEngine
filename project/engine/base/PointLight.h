#pragma once
#include "Vector4.h"
#include "Vector3.h"

/// <summary>
/// ポイントライト構造体
/// </summary>
struct PointLight
{
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius;
	float decay;
	float padding[2];
};