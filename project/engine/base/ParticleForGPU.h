#pragma once
#include "Matrix4x4.h"
#include "Vector4.h"

/// <summary>
/// GPUに送るパーティクルの構造体
/// </summary>
struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};