#pragma once

class GameTimer {
public:
	/// <summary>
	/// 初期化関数
	/// </summary>
	/// <param name="duration">設定時間</param>
	void Initialize(float duration);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="deltaTime">経過時間</param>
	void Update(float deltaTime);

	/// <summary>
	/// 終了検知関数
	/// </summary>
	/// <returns>タイマー計測が終了しているかどうか</returns>
	bool IsEnd() const;

	/// <summary>
	/// 現在の経過時間を返す関数
	/// </summary>
	/// <returns>現在の経過時間</returns>
	float GetTimer() const { return timer_; }

private:
	float timer_ = 0.0f;    // 経過時間
	float duration_ = 0.0f; // 設定する時間
};