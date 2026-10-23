// ButtonGroup.cpp
#include "ButtonGroup.h"
#include <algorithm>
#include <cmath>

void ButtonGroup::Initialize(Layout layout, TinyEngine::AudioManager* audioManager) {
	layout_ = layout;
	audio_ = audioManager;
	selectedIndex_ = 0;
	stickInUse_ = false;
	isDecided_ = false;
	buttons_.clear();
}

void ButtonGroup::Add(EngineContext* ctx, TinyEngine::Font* font, const std::wstring& label, const Vector2& center, std::function<void()> onDecide) {
	buttons_.emplace_back();
	buttons_.back().Initialize(ctx, font, label, center, std::move(onDecide));
}

void ButtonGroup::Update(DirectInput* input, GamePad* gamePad) {
	if (buttons_.empty()) {
		return;
	}

	// 決定後は入力を受け付けない
	if (!isDecided_) {
		const int count = static_cast<int>(buttons_.size());

		if (int move = PollMove(input, gamePad); move != 0) {
			selectedIndex_ = (selectedIndex_ + move + count) % count;
			if (audio_) {
				audio_->PlaySE("Select", 0.5f);
			}
		}

		if (PollDecide(input, gamePad)) {
			isDecided_ = true;
			if (audio_) {
				audio_->PlaySE("Decide", 0.5f);
			}
			buttons_[selectedIndex_].Execute();
		}
	}

	for (int i = 0; i < static_cast<int>(buttons_.size()); ++i) {
		buttons_[i].Update(i == selectedIndex_);
	}
}

void ButtonGroup::Draw() {
	for (auto& button : buttons_) {
		button.Draw();
	}
}

void ButtonGroup::DrawImGui() {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("ButtonGroup")) {
		for (size_t i = 0; i < buttons_.size(); ++i) {
			std::string label = "Button " + std::to_string(i);
			buttons_[i].DrawImGui(label.c_str());
		}
		ImGui::TreePop();
	}
#endif
}

int ButtonGroup::PollMove(DirectInput* input, GamePad* gamePad) {
	const bool horizontal = (layout_ == Layout::Horizontal);
	int move = 0;

	// キーボード
	if (input->KeyTriggered(horizontal ? DIK_A : DIK_W)) {
		move -= 1;
	}
	if (input->KeyTriggered(horizontal ? DIK_D : DIK_S)) {
		move += 1;
	}

	// ゲームパッド
	if (gamePad && gamePad->GetState().connected) {
		const auto& pad = gamePad->GetState();

		// スティック
		const float axis = horizontal ? pad.axes.lx : pad.axes.ly;
		if (!stickInUse_) {
			if (axis > 0.9f) {
				move += 1;
				stickInUse_ = true;
			} else if (axis < -0.9f) {
				move -= 1;
				stickInUse_ = true;
			}
		}

		// ニュートラルに戻ったら再入力可能にする
		if (std::abs(axis) < 0.3f) {
			stickInUse_ = false;
		}

		// 十字キー
		const auto& pressed = pad.buttonsPressed;
		if (horizontal) {
			move += static_cast<int>(pressed.dpadRight) - static_cast<int>(pressed.dpadLeft);
		} else {
			move += static_cast<int>(pressed.dpadDown) - static_cast<int>(pressed.dpadUp);
		}
	}

	return std::clamp(move, -1, 1);
}

bool ButtonGroup::PollDecide(DirectInput* input, GamePad* gamePad) const {
	if (input->KeyTriggered(DIK_SPACE)) {
		return true;
	}

	return gamePad && gamePad->GetState().connected && gamePad->GetState().buttonsPressed.a;
}