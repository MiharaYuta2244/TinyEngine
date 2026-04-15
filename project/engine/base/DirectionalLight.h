#pragma once
#include "Vector3.h"
#include "Vector4.h"

/// <summary>
/// GPUに送る方向光源のパラメータの構造体
/// </summary>
struct DirectionalLight
{
	Vector4 color; //!< ライトの色
	Vector3 direction; //!< ライトの向き
	float intensity; //!< 輝度
};