#pragma once
#include <string>

/// <summary>
/// GPUに送るマテリアルデータの構造体
/// </summary>
struct MaterialData
{
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};