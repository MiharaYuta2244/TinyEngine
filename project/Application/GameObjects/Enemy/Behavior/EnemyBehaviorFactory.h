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

	// 敵の名前一覧
	static constexpr const char* kTypeNames[] = {"Normal", "Shotgun", "Bomber", "Assault"};
	static_assert(std::size(kTypeNames) == static_cast<size_t>(EnemyType::Count), "kTypeNamesの数がEnemyTypeと一致していません");
};
