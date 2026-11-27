#pragma once
#include "EnemyState.h"
#include <array>
#include <cstddef>

namespace TinyEngine {
class Object3d;
}

/// <summary>
/// 敵が再生するアニメーションの種類
/// </summary>
enum class EnemyAnim {
	Idle,          // 待機
	Walk,          // 通常歩き
	VigilanceWalk, // 警戒歩き
	Count,
};

/// <summary>
/// 敵の状態に応じてアニメーションクリップを切り替えるクラス
/// モデルは差し替えず、クリップだけを切り替える
/// </summary>
class EnemyAnimator {
public:
	// クリップ切り替え時のブレンド時間
	static constexpr float kDefaultBlend = 0.15f;

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="object3d">対象のObject3d</param>
	void Initialize(TinyEngine::Object3d* object3d);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="state">敵の状態</param>
	/// <param name="isMoving">このフレームに移動したか</param>
	void Update(EnemyState state, bool isMoving);

	/// <summary>
	/// 指定したアニメーションを再生
	/// </summary>
	void Play(EnemyAnim anim, float blendDuration = kDefaultBlend);

	// 現在のアニメーション
	EnemyAnim GetCurrent() const { return current_; }

	/// <summary>
	/// 状態と移動有無から再生すべきアニメーションを決める
	/// </summary>
	static EnemyAnim Select(EnemyState state, bool isMoving);

private:
	// EnemyAnimと対応するクリップファイル名
	static constexpr std::array<const char*, static_cast<std::size_t>(EnemyAnim::Count)> kClipFiles = {
	    "HumanIdle.gltf",
	    "HumanNormalWalk.gltf",
	    "HumanVigilanceWalk.gltf",
	};

private:
	TinyEngine::Object3d* object3d_ = nullptr;
	EnemyAnim current_ = EnemyAnim::Idle;
	bool hasPlayed_ = false; // 一度でも再生したか
};
