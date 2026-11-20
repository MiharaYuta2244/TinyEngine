#include "InputBindings.h"
#include <utility>

InputBindings& InputBindings::Get() {
	static InputBindings value;
	return value;
}

void InputBindings::ResetKeyboard() {
	keyboard = InputBindings{}.keyboard;
}

void InputBindings::ResetPad() {
	pad = InputBindings{}.pad;
}

void InputBindings::Assign(bool controller, int action, int code) {
	auto& bindings = controller ? pad : keyboard;
	for (int i = 0; i < kActionCount; ++i) {
		if (i != action && bindings[i] == code) {
			std::swap(bindings[i], bindings[action]);
			return;
		}
	}
	bindings[action] = code;
}

bool InputBindings::ValidKeyboard(int code) {
	return (code > 0 && code < 256 && code != DIK_ESCAPE) || (code >= kMouseBase && code < kMouseBase + 8);
}

bool InputBindings::PadDown(int code, const GamepadState& state) {
	if (!state.connected) return false;
	const auto& b = state.buttons;
	switch (static_cast<PadBinding>(code)) {
	case PadBinding::A: return b.a;
	case PadBinding::B: return b.b;
	case PadBinding::X: return b.x;
	case PadBinding::Y: return b.y;
	case PadBinding::LB: return b.lb;
	case PadBinding::RB: return b.rb;
	case PadBinding::LT: return state.axes.lt > 0.5f;
	case PadBinding::RT: return state.axes.rt > 0.5f;
	case PadBinding::Start: return b.start;
	case PadBinding::LS: return b.ls;
	case PadBinding::RS: return b.rs;
	case PadBinding::Up: return b.dpadUp;
	case PadBinding::Down: return b.dpadDown;
	case PadBinding::Left: return b.dpadLeft;
	case PadBinding::Right: return b.dpadRight;
	default: return false;
	}
}

bool InputBindings::KeyboardDown(int code, DirectInput* input) {
	return code >= kMouseBase ? input->MouseButtonDown(static_cast<uint8_t>(code - kMouseBase)) : input->KeyDown(static_cast<uint8_t>(code));
}

bool InputBindings::Down(GameAction action, DirectInput* input, GamePad* gamePad) const {
	const int i = static_cast<int>(action);
	return KeyboardDown(keyboard[i], input) || (gamePad && PadDown(pad[i], gamePad->GetState()));
}

bool InputBindings::Pressed(GameAction action, DirectInput* input, GamePad* gamePad) const {
	const int i = static_cast<int>(action);
	const int code = keyboard[i];
	bool pressed = code >= kMouseBase ? input->MouseButtonTriggered(static_cast<uint8_t>(code - kMouseBase)) : input->KeyTriggered(static_cast<uint8_t>(code));
	return pressed || (gamePad && PadDown(pad[i], gamePad->GetState()) && !previousPad_[i]);
}

void InputBindings::EndFrame(GamePad* gamePad) {
	for (int i = 0; i < kActionCount; ++i) {
		previousPad_[i] = gamePad && PadDown(pad[i], gamePad->GetState());
	}
}

std::wstring InputBindings::Name(bool controller, int code) {
	if (controller) {
		static const std::array<const wchar_t*, 15> names = {L"A", L"B", L"X", L"Y", L"LB", L"RB", L"LT", L"RT", L"Start", L"LS", L"RS", L"十字キー上", L"十字キー下", L"十字キー左", L"十字キー右"};
		return names[code];
	}
	if (code >= kMouseBase) {
		static const std::array<const wchar_t*, 8> names = {L"マウス左", L"マウス右", L"マウス中央", L"マウス4", L"マウス5", L"マウス6", L"マウス7", L"マウス8"};
		return names[code - kMouseBase];
	}
	wchar_t name[128] = {};
	const UINT scanCode = static_cast<UINT>(code & 0x7f);
	const LONG flags = static_cast<LONG>((scanCode << 16) | ((code & 0x80) ? (1u << 24) : 0));
	if (GetKeyNameTextW(flags, name, 128) > 0) return name;
	return L"キー " + std::to_wstring(code);
}
