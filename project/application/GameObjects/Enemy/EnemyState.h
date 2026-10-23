#pragma once

/// <summary>
/// 敵の状態
/// </summary>
enum class EnemyState {
	Normal,    // 徘徊、通常状態
	Vigilance, // 警戒、プレイヤー追跡状態
	Hold,      // プレイヤーに拘束されている状態
	Down,      // ひるみ状態
};
