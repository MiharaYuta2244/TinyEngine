#pragma once
class FadeManager;

/// <summary>
/// フェード状態の基底インターフェース
/// </summary>
class IFadeState {
public:
	virtual ~IFadeState() = default;

	/// <summary>
	/// 状態遷移時に呼ばれる処理
	/// </summary>
	/// <param name="manager">フェードマネージャーのポインタ</param>
	virtual void Enter(FadeManager* manager) {}

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="manager">フェードマネージャーのポインタ</param>
	/// <param name="deltaTime">経過時間</param>
	virtual void Update(FadeManager* manager, float deltaTime) = 0;

	/// <summary>
	/// 状態から抜ける瞬間に呼ばれる処理
	/// </summary>
	/// <param name="manager">フェードマネージャーのポインタ</param>
	virtual void Exit(FadeManager* manager) {}
};
