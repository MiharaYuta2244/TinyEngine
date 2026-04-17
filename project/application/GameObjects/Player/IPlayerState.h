#pragma once

namespace Player {
/// <summary>
/// プレイヤーの状態の基底クラス
/// </summary>
class IPlayerState {
public:
	virtual ~IPlayerState() = default;

	// 状態切り替え時に一度だけ呼ばれる関数
	virtual void Enter() {};

	// 更新処理
	virtual void Update(float deltaTime) = 0;

	// 別の状態に切り替わる直前に呼ばれる
	virtual void Exit() {};
};

} // namespace Player