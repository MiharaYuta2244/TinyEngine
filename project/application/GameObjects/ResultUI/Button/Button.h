#pragma once
#include "Font.h"
#include "TextSprite.h"
#include <functional>
#include <memory>
#include <string>

/// <summary>
/// フォント描画のボタン1個分
/// </summary>
class Button {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, TinyEngine::Font* font, const std::wstring& label, const Vector2& center, std::function<void()> onDecide);

	// 更新処理
	void Update(bool isSelected);

	// 描画処理
	void Draw();

	// ImGui描画処理
	void DrawImGui(const char* label);

	// 座標のSetter
	void SetPosition(const Vector2& position) {
		position_ = position;
		if (text_) {
			text_->SetPosition(position_);
		}
	}

	// 座標のGetter
	const Vector2& GetPosition() const { return position_; }

	// コールバック関数呼び出し処理
	void Execute() const {
		if (onDecide_) {
			onDecide_();
		}
	}

private:
	// テキスト
	std::unique_ptr<TinyEngine::TextSprite> text_;

	// ボタンを押したときのコールバック
	std::function<void()> onDecide_;

	// 座標
	Vector2 position_ = {0.0f, 0.0f};
};