#pragma once
#include "DirectInput.h"
#include "GamePad.h"
#include <array>
#include <string>

enum class GameAction { Up, Down, Left, Right, Attack, Grab, Pause, Count };
enum class PadBinding { A, B, X, Y, LB, RB, LT, RT, Start, LS, RS, Up, Down, Left, Right, Count };

// ゲーム操作の割り当てと入力状態を管理する
class InputBindings {
public:
	// 各入力機器の割り当てを初期値に戻す
	void ResetKeyboard();
	void ResetPad();

	// 指定した入力が使用済みなら、割り当てを入れ替える
	void Assign(bool controller, int action, int code);

	// キャンセル用のEscapeを除き、割り当て可能な入力か確認する
	static bool ValidKeyboard(int code);
	static bool PadDown(int code, const GamepadState& state);
	static bool KeyboardDown(int code, DirectInput* input);

	// 操作に対応する入力の押下状態と、押した瞬間を取得する
	bool Down(GameAction action, DirectInput* input, GamePad* gamePad) const;
	bool Pressed(GameAction action, DirectInput* input, GamePad* gamePad) const;

	// 設定画面を含むシーン更新の後に、毎フレーム呼び出す
	void EndFrame(GamePad* gamePad);

	// 設定画面に表示するキー・ボタン名を取得する
	static std::wstring Name(bool controller, int code);

public:
	static constexpr int kActionCount = static_cast<int>(GameAction::Count);
	static constexpr int kMouseBase = 256;
	static InputBindings& Get();
	std::array<int, kActionCount> keyboard = {DIK_W, DIK_S, DIK_A, DIK_D, kMouseBase, kMouseBase + 1, DIK_TAB};
	std::array<int, kActionCount> pad = {11, 12, 13, 14, 7, 6, 8};

private:
	std::array<bool, kActionCount> previousPad_{};
};
