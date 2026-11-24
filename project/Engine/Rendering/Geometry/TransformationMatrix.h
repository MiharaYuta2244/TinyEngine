#pragma once
#include "Matrix4x4.h"

/// <summary>
/// 変換行列構造体
/// </summary>
struct TransformationMatrix
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 WorldInverseTranspose;
};