#pragma once
#include "Vector3.h"

/// <summary>
/// エディターからドラッグ&ドロップで生成できるオブジェクトの種類
/// </summary>
enum class SpawnType { Wall, Door, Glass, Cage, HealArea, Soap, Enemy };

/// <summary>
/// 生成リクエスト
/// </summary>
struct SpawnRequest {
	SpawnType type;
	Vector3 position;
};