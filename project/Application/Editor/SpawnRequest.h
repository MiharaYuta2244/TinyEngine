#pragma once
#include "Vector3.h"

/// <summary>
/// エディターからドラッグ&ドロップで生成できるオブジェクトの種類
/// </summary>
enum class SpawnType { Wall, Door, Glass, Cage, Soap, Enemy, Barricade, Decoration, Molotov, TreasureChest };

/// <summary>
/// 生成リクエスト
/// </summary>
struct SpawnRequest {
	SpawnType type;
	Vector3 position;
};
