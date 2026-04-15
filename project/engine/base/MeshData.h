#pragma once
#include "VertexData.h"
#include <vector>

/// <summary>
/// GPUに送るメッシュデータの構造体
/// </summary>
class MeshData {
public:
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};
