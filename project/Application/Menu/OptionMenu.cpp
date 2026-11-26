#include "OptionMenu.h"
#include "AudioManager.h"
#include "InputBindings.h"
#include "Utility/SettingsStorage.h"
#include <algorithm>
#include <cmath>
#include <utility>

void OptionMenu::Initialize(EngineContext* ctx, PostEffectSettings* effects, std::function<void()> onClose) {
	effects_ = effects;
	ctx_ = ctx;
	onClose_ = std::move(onClose);

	// 各階層の表示位置とスクロール範囲を統一する
	for (int i = 0; i < static_cast<int>(menus_.size()); ++i) {
		auto& menu = menus_[i];
		menu = std::make_unique<MenuList>();

		// 項目と操作説明を日本語で表示するため、漢字を含む文字セットを使用する。
		menu->Initialize(ctx, L"Dela Gothic One", 30, true);
		menu->SetStartPos({400.0f, 290.0f});
		menu->SetOffsetY(46.0f);
		menu->SetVisibleCount(7);
	}

	// 最上位では設定の種類を選び、詳細は対応する子階層で変更する
	auto& root = Menu(Page::Root);
	root.AddItem(L"ポストエフェクト設定", [this]() { Enter(Page::Effects); });
	root.AddItem(L"入力設定", [this]() { Enter(Page::Input); });
	root.AddItem(L"オーディオ設定", [this]() { Enter(Page::Audio); });
	root.AddItem(L"戻る", [this]() { onClose_(); });

	// チェック項目は共有設定を直接参照し、変更をゲーム側にも反映する
	auto& pfx = Menu(Page::Effects);
	pfx.AddToggleItem(L"周辺減光", &effects->vignetteEnabled);
	pfx.AddToggleItem(L"放射状ぼかし", &effects->radialBlurEnabled);
	pfx.AddToggleItem(L"ノイズ", &effects->glitchEnabled);
	pfx.AddToggleItem(L"走査線", &effects->scanlineEnabled);
	pfx.AddToggleItem(L"ブラウン管風", &effects->barrelDistortionEnabled);
	pfx.AddToggleItem(L"ぼかし", &effects->blurEnabled);
	pfx.AddToggleItem(L"死亡時のエフェクト", &effects->deathEffectEnabled);
	pfx.AddToggleItem(L"輪郭線", &effects->outlineEnabled);
	pfx.AddItem(L"戻る", [this]() { Enter(Page::Root); });

	auto& input = Menu(Page::Input);
	input.AddItem(L"キーボード・マウス", [this]() { Enter(Page::Keyboard); });
	input.AddItem(L"ゲームパッド", [this]() { Enter(Page::Gamepad); });
	input.AddItem(L"戻る", [this]() { Enter(Page::Root); });

	// 入力機器ごとに操作一覧を作り、選択した操作だけを割り当て待ちにする
	for (bool controller : {false, true}) {
		auto& menu = Menu(controller ? Page::Gamepad : Page::Keyboard);
		for (int i = 0; i < InputBindings::kActionCount; ++i) {
			menu.AddItem(
			    ActionName(i),
			    [this, i, controller]() {
				    captureAction_ = i;
				    capturePad_ = controller;
				    captureArmed_ = false;
			    },
			    true);
			// 入れ替え後の割り当てや入力待ちの状態を、表示時に取得する
			menu.SetDynamicLabel([this, i, controller]() {
				if (captureAction_ == i && capturePad_ == controller)
					return ActionName(i) + L" : [入力してください]";
				const auto& bindings = controller ? InputBindings::Get().pad : InputBindings::Get().keyboard;
				return ActionName(i) + L" : " + InputBindings::Name(controller, bindings[i]);
			});
		}

		menu.AddItem(
		    L"初期値に戻す",
		    [controller]() {
			    if (controller)
				    InputBindings::Get().ResetPad();
			    else
				    InputBindings::Get().ResetKeyboard();
		    },
		    true);
		menu.AddItem(L"戻る", [this]() { Enter(Page::Input); });
	}

	// スライダーの操作を種類ごとの全体音量に即時反映する。
	auto& audio = Menu(Page::Audio);
	audio.AddSliderItem(L"BGM", []() { return TinyEngine::AudioManager::Get().GetBGMVolume(); }, [](float value) { TinyEngine::AudioManager::Get().SetBGMVolume(value); });
	audio.AddSliderItem(L"SE", []() { return TinyEngine::AudioManager::Get().GetSEVolume(); }, [](float value) { TinyEngine::AudioManager::Get().SetSEVolume(value); });
	audio.AddItem(L"戻る", [this]() { Enter(Page::Root); });

	font_ = std::make_unique<TinyEngine::Font>();
	font_->Initialize(ctx, L"Dela Gothic One", 20, TinyEngine::Font::MakeJapaneseCharSet(true));
	heading_ = std::make_unique<TinyEngine::TextSprite>();
	heading_->Initialize(ctx, font_.get(), L"オプション");
	heading_->SetPosition({400.0f, 230.0f});
	help_ = std::make_unique<TinyEngine::TextSprite>();
	help_->Initialize(ctx, font_.get(), L"");
	help_->SetPosition({110.0f, 650.0f});
	static const std::array<const char*, 6> keys = {"option.root", "option.effects", "option.input", "option.audio", "option.keyboard", "option.gamepad"};
	for (int i = 0; i < static_cast<int>(menus_.size()); ++i)
		menus_[i]->BindLayout(keys[i]);
}

void OptionMenu::ResetState() {
	captureAction_ = -1;
	Enter(Page::Root);
}

void OptionMenu::Update(DirectInput* input, GamePad* gamePad, float deltaTime) {
	// ImGuiの数値欄を操作している間は、同じキーをメニューにも渡さない。
	if (!ctx_->menuKeyboardAvailable)
		return;

	// 割り当て用の入力が階層移動や項目選択にも使われることを防ぐ
	if (captureAction_ >= 0) {
		Capture(input, gamePad);
		return;
	}

	const bool cancel = input->KeyTriggered(DIK_ESCAPE) || (gamePad && gamePad->GetState().connected && gamePad->GetState().buttonsPressed.b);
	if (cancel) {
		if (page_ == Page::Root)
			onClose_();
		else
			Enter(page_ == Page::Keyboard || page_ == Page::Gamepad ? Page::Input : Page::Root);
		return;
	}

	// 操作前後の設定を比較し、実際に変更されたときだけ保存する
	const auto before = SettingsStorage::Serialize(*effects_);
	Menu(page_).Update(input, gamePad, deltaTime);

	if (SettingsStorage::Serialize(*effects_) != before) {
		Save();
	}
}

void OptionMenu::Draw() {
	// 現在の階層と入力待ち状態に応じて、見出しと操作説明を切り替える
	Menu(page_).Draw();
	static const std::array<const wchar_t*, 6> titles = {
	    L"オプション", L"オプション / ポストエフェクト", L"オプション / 入力設定", L"オプション / オーディオ設定", L"入力設定 / キーボード・マウス", L"入力設定 / ゲームパッド"};
	static const std::array<const char*, 6> keys = {"option.root", "option.effects", "option.input", "option.audio", "option.keyboard", "option.gamepad"};
	const auto& layout = MenuLayoutStore::Get().Layout(keys[static_cast<int>(page_)]);
	heading_->SetPosition(layout.headingPosition);
	heading_->SetScale(layout.headingScale);
	heading_->SetColor(layout.colors.heading);
	help_->SetPosition(layout.helpPosition);
	help_->SetScale(layout.helpScale);
	help_->SetColor(layout.colors.help);
	heading_->SetText(titles[static_cast<int>(page_)]);
	heading_->Update();
	heading_->Draw();
	std::wstring hint = L"W/S・上下：選択　Enter/Space・A：決定　Esc・B：戻る";

	if (captureAction_ >= 0) {
		hint = capturePad_ ? L"一度ボタンを離してから入力してください　Esc・Back：キャンセル" : L"一度キーを離してから入力してください　Esc：キャンセル";
	} else if (page_ == Page::Audio) {
		hint = L"マウスで音量調整　A/D・左右：1%ずつ変更（長押し対応）　Esc・B：戻る";
	} else if (page_ == Page::Gamepad) {
		hint = L"変更する操作を選択　スティック：移動・照準　重複時は入れ替え";
	} else if (page_ == Page::Keyboard) {
		hint = L"変更する操作を選び、キーかマウスボタンを入力　重複時は入れ替え";
	}

	if (saveFailed_) {
		hint = L"設定の保存に失敗しました。今回のプレイには反映されます。";
	}

	help_->SetText(hint);
	help_->Update();
	help_->Draw();
}

MenuList& OptionMenu::Menu(Page page) { return *menus_[static_cast<int>(page)]; }

void OptionMenu::Enter(Page page) {
	page_ = page;
	Menu(page_).ResetState();
}

std::wstring OptionMenu::ActionName(int action) {
	static const std::array<const wchar_t*, 7> names = {L"上へ移動", L"下へ移動", L"左へ移動", L"右へ移動", L"攻撃 / 投げる", L"掴む", L"ポーズ"};
	return names[action];
}

bool OptionMenu::PreviewLayout(const std::string& key) {
	static const std::array<const char*, 6> keys = {"option.root", "option.effects", "option.input", "option.audio", "option.keyboard", "option.gamepad"};
	for (int i = 0; i < static_cast<int>(keys.size()); ++i) {
		if (key == keys[i]) {
			if (page_ != static_cast<Page>(i)) {
				captureAction_ = -1;
				Enter(static_cast<Page>(i));
			}
			return true;
		}
	}
	return false;
}

void OptionMenu::Save() { saveFailed_ = !SettingsStorage::Save(*effects_); }

void OptionMenu::Capture(DirectInput* input, GamePad* gamePad) {
	// キャンセル時は現在の割り当てを変更しない
	if (input->KeyTriggered(DIK_ESCAPE) || (capturePad_ && gamePad && gamePad->GetState().buttonsPressed.back)) {
		captureAction_ = -1;
		return;
	}

	// 入力機器に応じて、現在押されているキーやボタンを探す
	int code = -1;
	if (capturePad_) {
		if (gamePad && gamePad->GetState().connected) {
			for (int i = 0; i < static_cast<int>(PadBinding::Count); ++i) {
				if (InputBindings::PadDown(i, gamePad->GetState())) {
					code = i;
					break;
				}
			}
			if (!captureArmed_ && gamePad->GetState().buttons.back)
				return;
		}
	} else {
		for (int i = 1; i < InputBindings::kMouseBase + 8; ++i) {
			if (InputBindings::ValidKeyboard(i) && InputBindings::KeyboardDown(i, input)) {
				code = i;
				break;
			}
		}
	}

	// 項目を決定したボタンが、そのまま新しい割り当てになることを防ぐ
	if (!captureArmed_) {
		if (code < 0)
			captureArmed_ = true;
		return;
	}

	if (code >= 0) {
		// 重複時の入れ替えは入力管理側へ任せ、確定した設定を保存する
		InputBindings::Get().Assign(capturePad_, captureAction_, code);
		captureAction_ = -1;
		Save();
	}
}
