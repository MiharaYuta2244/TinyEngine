#include "PauseScene.h"
#include "Fade/SceneTransition.h"
#include "SceneManager.h"
#include "InputBindings.h"

using namespace TinyEngine;

void PauseScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;
	optionsOpen_ = false;

	// メニューの生成&初期化
	menuList_ = std::make_unique<MenuList>();
	menuList_->Initialize(ctx.engineContext, L"Dela Gothic One", 30, true);

	// メニュー項目の追加
	menuList_->AddItem(L"再開", [this]() { ctx_.sceneTransition->PopScene(); });
	menuList_->AddItem(L"オプション", [this]() { optionsOpen_ = true; optionMenu_->ResetState(); });
	menuList_->AddItem(L"タイトルへ戻る", [this]() { ctx_.sceneTransition->ChangeScene("Title", FadeStyle::Logo); });
	menuList_->BindLayout("pause");
	optionMenu_ = std::make_unique<OptionMenu>();
	optionMenu_->Initialize(ctx.engineContext, &ctx_.sceneManager->GetCommonData()->postEffectSettings,
	    [this]() { optionsOpen_ = false; menuList_->ResetState(); });

	// 背景スプライト生成&初期化
	bgSprite_ = std::make_unique<Sprite>();
	bgSprite_->Initialize(ctx.engineContext, "white.png");
	bgSprite_->SetColor({0, 0, 0, 0.8f});
	bgSprite_->SetSize({1280.0f, 720.0f});

	// フォント生成&初期化
	font_ = std::make_unique<Font>();
	font_->Initialize(ctx.engineContext, L"Dela Gothic One", 20, Font::MakeJapaneseCharSet(true));

	// テキスト生成&初期化
	textSprite_ = std::make_unique<TextSprite>();
	textSprite_->Initialize(ctx.engineContext, font_.get(), L"ポーズ");
	textSprite_->SetPosition({518.0f, 50.0f});
	textSprite_->SetColor({0.5f, 0.5f, 0.0f, 1.0f});
}

void PauseScene::Update() {
	if (optionsOpen_) {
		optionMenu_->Update(ctx_.keyboard, ctx_.gamePad, ctx_.timeManager->GetDeltaTime());
		bgSprite_->Update();
		return;
	}

	const auto& pad = ctx_.gamePad->GetState();
	const bool menuInput = ctx_.keyboard->KeyTriggered(DIK_SPACE) || ctx_.keyboard->KeyTriggered(DIK_RETURN)
	    || ctx_.keyboard->KeyTriggered(DIK_W) || ctx_.keyboard->KeyTriggered(DIK_S)
	    || ctx_.keyboard->KeyTriggered(DIK_UP) || ctx_.keyboard->KeyTriggered(DIK_DOWN)
	    || pad.buttonsPressed.a || pad.buttonsPressed.dpadUp || pad.buttonsPressed.dpadDown;
	if ((!menuInput && InputBindings::Get().Pressed(GameAction::Pause, ctx_.keyboard, ctx_.gamePad)) || ctx_.keyboard->KeyTriggered(DIK_ESCAPE) || pad.buttonsPressed.b) {
		ctx_.sceneTransition->PopScene();
		return;
	}

	// 背景更新
	bgSprite_->Update();

	// メニューの更新
	menuList_->Update(ctx_.keyboard, ctx_.gamePad, ctx_.timeManager->GetDeltaTime());

	// ポーズテキストの更新
	textSprite_->Update();
}

void PauseScene::Draw() {
	// 背景描画
	bgSprite_->Draw();

	// メニューの描画
	if (optionsOpen_) optionMenu_->Draw();
	else menuList_->Draw();

	// ポーズテキスト描画
	if (!optionsOpen_) {
		const auto& layout = MenuLayoutStore::Get().Layout("pause");
		textSprite_->SetPosition(layout.headingPosition);
		textSprite_->SetScale(layout.headingScale);
		textSprite_->SetColor(layout.colors.heading);
		textSprite_->Update(); textSprite_->Draw();
	}
}

void PauseScene::Finalize() {}
