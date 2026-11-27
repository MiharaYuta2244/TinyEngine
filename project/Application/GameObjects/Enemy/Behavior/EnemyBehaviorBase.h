#pragma once
#include "EnemyBehavior.h"

/// <summary>
/// 敵の共通処理をまとめた基底クラス
/// </summary>
class EnemyBehaviorBase : public IEnemyBehavior {
protected:
	/// <summary>
	/// 弾の発射処理
	/// </summary>
	/// <param name="c">発射情報</param>
	/// <param name="yawOffset">発射方向からの左右のずれ</param>
	static void FireBullet(const ShotContext& c, float yawOffset = 0.0f);
};
