#pragma once
#include "MeshData.h"
#include <vector>

/// <summary>
/// メッシュデータを生成するインターフェース
/// </summary>
class IMeshGenerator
{
public:
	virtual ~IMeshGenerator() = default;
	virtual MeshData GenerateMeshData() const = 0;
};