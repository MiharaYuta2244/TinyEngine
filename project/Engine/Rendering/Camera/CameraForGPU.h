#pragma once
#include "Vector3.h"

/// <summary>
/// GPUに送るカメラのパラメータの構造体
/// </summary>
struct CameraForGPU
{
	Vector3 worldPosition;
};