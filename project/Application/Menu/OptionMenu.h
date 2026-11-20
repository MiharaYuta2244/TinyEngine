#pragma once
#include "MenuList.h"
#include "PostEffectSetting.h"
#include <array>
#include <functional>
#include <memory>
#include <string>

/// <summary>
/// タイトル画面とポーズ画面で共通利用する階層型の設定メニュー
/// </summary>
class OptionMenu {
public:
	// 初期化関数
	void Initialize(EngineContext* ctx, PostEffectSettings* effects, std::function<void()> onClose);

	// 割り当て待ちを解除し、設定の最上位階層へ戻す
	void ResetState();

	// 更新関数
	void Update(DirectInput* input, GamePad* gamePad, float deltaTime);

	// 描画関数
	void Draw();
	// 起動画面の編集機能から、指定した設定階層を表示する。
	bool PreviewLayout(const std::string& key);

private:
	enum class Page { Root, Effects, Input, Audio, Keyboard, Gamepad };

	// 指定した階層のメニューを取得する。
	MenuList& Menu(Page page);

	// 移動先の決定ロックを解除し、操作可能な状態で階層を切り替える
	void Enter(Page page);
	static std::wstring ActionName(int action);

	// 設定を保存し、失敗時は操作説明欄に通知する
	void Save();

	// 決定時の入力が離されるのを待ってから、新しい割り当てを受け付ける
	void Capture(DirectInput* input, GamePad* gamePad);

private:
	Page page_ = Page::Root;
	std::array<std::unique_ptr<MenuList>, 6> menus_;
	PostEffectSettings* effects_ = nullptr; // シーン間で共有される設定を参照する用
	std::function<void()> onClose_;
	int captureAction_ = -1;    // -1なら通常操作、それ以外は指定操作の割り当て待ち
	bool capturePad_ = false;   // ゲームパッドの割り当て待ちかどうか
	bool captureArmed_ = false; // 決定時に押されていた入力が離されたかどうか
	bool saveFailed_ = false;
	EngineContext* ctx_ = nullptr;
	std::unique_ptr<TinyEngine::Font> font_;
	std::unique_ptr<TinyEngine::TextSprite> heading_, help_;
};
