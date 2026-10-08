#pragma once
#include "BaseScene.h"
#include "Font.h"
#include "Menu/MenuList.h"
#include "Sprite.h"
#include "TextSprite.h"

/// <summary>
/// ポーズシーン
/// </summary>
class PauseScene : public BaseScene {
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// メニュー
	std::unique_ptr<MenuList> menuList_;

	// 背景
	std::unique_ptr<TinyEngine::Sprite> bgSprite_;

	// フォント
	std::unique_ptr<TinyEngine::Font> font_;

	// テキスト
	std::unique_ptr<TinyEngine::TextSprite> textSprite_;
};
