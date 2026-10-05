#pragma once
#include "EnemyBehavior.h"

/// <summary>
/// 敵の種類の分岐を一か所にまとめたクラス
/// </summary>
class EnemyBehaviorFactory {
public:
	/// <summary>
	/// 指定した敵の種類を返す関数
	/// </summary>
	/// <param name="type">敵の種類</param>
	/// <returns></returns>
	static std::unique_ptr<IEnemyBehavior> Create(EnemyType type);
};
