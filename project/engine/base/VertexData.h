#pragma once
#include "Vector2.h"
#include "Vector4.h"
#include "Vector3.h"

/// <summary>
/// 頂点データ構造体
/// </summary>
struct VertexData
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};