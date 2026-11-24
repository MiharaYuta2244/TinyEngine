#pragma once

#include "Vector3.h"
#include "Vector4.h"

/// <summary>
/// スポットライト構造体
/// </summary>
struct SpotLight
{
	Vector4 color;
	Vector3 position;
	float intensity;
	Vector3 direction;
	float distance;
	float decay;
	float cosAngle;
	float padding[2];
};