#include "Button.h"

namespace {
const Vector4 kNormalColor = {1.0f, 1.0f, 1.0f, 1.0f};
const Vector4 kSelectedColor = {1.0f, 1.0f, 0.0f, 1.0f};
constexpr float kNormalScale = 1.0f;
constexpr float kSelectedScale = 1.2f;
} // namespace

void Button::Initialize(EngineContext* ctx, TinyEngine::Font* font, const std::wstring& label, const Vector2& center, std::function<void()> onDecide) {
	onDecide_ = std::move(onDecide);
	position_ = center;

	text_ = std::make_unique<TinyEngine::TextSprite>();
	text_->Initialize(ctx, font, label);
	text_->SetAnchorPoint({0.5f, 0.5f});
	text_->SetPosition(center);
}

void Button::Update(bool isSelected) {
	text_->SetColor(isSelected ? kSelectedColor : kNormalColor);
	text_->SetScale(isSelected ? kSelectedScale : kNormalScale);
	text_->Update();
}

void Button::Draw() { text_->Draw(); }

void Button::DrawImGui(const char* label) {
#ifdef USE_IMGUI
	Vector2 pos = position_;
	if (ImGui::DragFloat2(label, &pos.x, 1.0f)) {
		SetPosition(pos);
	}
#endif
}