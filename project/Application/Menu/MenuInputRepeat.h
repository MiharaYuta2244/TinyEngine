#pragma once

/// <summary>
/// 押した瞬間に1回、待ち時間の後は一定間隔で方向入力を繰り返す
/// </summary>
class MenuInputRepeat {
public:
	// 更新関数
	int Update(int direction, float deltaTime, float delay, float interval);

	// 階層移動時は、押しっぱなしの入力を引き継がないようにする。
	void Reset(bool waitForRelease = false);
private:
	int direction_ = 0;
	float remaining_ = 0.0f;
	bool waitForRelease_ = false;
};
