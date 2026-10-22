#include "NextStageButton.h"

void NextStageButton::Initialize(EngineContext* ctx) {
	IButton::Initialize(ctx);

	// 個別のスプライト設定
	sprite_->Initialize(ctx, "NextStage.png");
	sprite_->SetPosition({750.0f, 550.0f});
}

void NextStageButton::Update(bool isSelect) { IButton::Update(isSelect); }

void NextStageButton::Draw() { IButton::Draw(); }

const ButtonStyle& NextStageButton::GetNormalStyle() const {
	static ButtonStyle style = {
	    {1, 1, 1, 1}, // 白
	    {408, 80}  // 通常サイズ
	};
	return style;
}

const ButtonStyle& NextStageButton::GetSelectedStyle() const {
	static ButtonStyle style = {
	    {1, 1, 0, 1}, // 黄色
	    {408 * 1.2f, 80 * 1.2f}  // 拡大
	};
	return style;
}
