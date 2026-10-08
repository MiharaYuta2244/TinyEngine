#include "PauseScene.h"

using namespace TinyEngine;

void PauseScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;

	// メニューの生成&初期化
	menuList_ = std::make_unique<MenuList>();
	menuList_->Initialize(ctx.engineContext);

	// メニュー項目の追加
	menuList_->AddItem("Resume", "Resume.png", [this]() { RequestScenePop(); });
	menuList_->AddItem("Title", "Title.png", [this]() { RequestSceneChange("Title"); });

	// 背景スプライト生成&初期化
	bgSprite_ = std::make_unique<Sprite>();
	bgSprite_->Initialize(ctx.engineContext, "white.png");
	bgSprite_->SetColor({0, 0, 0, 0.8f});
	bgSprite_->SetSize({1280.0f, 720.0f});

	// フォント生成&初期化
	font_ = std::make_unique<Font>();
	font_->Initialize(ctx.engineContext, L"Dela Gothic One", 64);

	// テキスト生成&初期化
	textSprite_ = std::make_unique<TextSprite>();
	textSprite_->Initialize(ctx.engineContext, font_.get(), L"Pause");
	textSprite_->SetPosition({518.0f, 50.0f});
	textSprite_->SetColor({0.5f, 0.5f, 0.0f, 1.0f});
}

void PauseScene::Update() {
	if (ctx_.keyboard->KeyTriggered(DIK_TAB) || ctx_.gamePad->GetState().buttonsPressed.start) {
		RequestScenePop();
	}

	// 背景更新
	bgSprite_->Update();

	// メニューの更新
	menuList_->Update(ctx_.keyboard, ctx_.gamePad, ctx_.timeManager->GetDeltaTime());

	// ポーズテキストの更新
	textSprite_->Update();

#ifdef USE_IMGUI
	ImGui::Begin("Pause Menu");

	// 音量調整
	static float volume = 1.0f;
	if (ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f)) {
		// サウンドマネージャー等に音量をセットする
	}

	// ゲームに戻る
	if (ImGui::Button("Resume")) {
		RequestScenePop();
	}

	// タイトルに戻る
	if (ImGui::Button("Back to Title")) {
		RequestSceneChange("Title");
	}

	ImGui::End();
#endif
}

void PauseScene::Draw() {
	// 背景描画
	bgSprite_->Draw();

	// メニューの描画
	menuList_->Draw();

	// ポーズテキスト描画
	textSprite_->Draw();
}

void PauseScene::Finalize() {}
