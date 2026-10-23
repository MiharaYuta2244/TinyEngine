#pragma once
#include "AudioManager.h"
#include "Button.h"
#include "DirectInput.h"
#include "GamePad.h"
#include <vector>

/// <summary>
/// ボタン群の選択・入力・決定を管理するクラス
/// </summary>
class ButtonGroup {
public:
	enum class Layout { Horizontal, Vertical };

	// 初期化処理
	void Initialize(Layout layout, TinyEngine::AudioManager* audioManager);

	// ボタンの追加
	void Add(EngineContext* ctx, TinyEngine::Font* font, const std::wstring& label, const Vector2& center, std::function<void()> onDecide);

	// 更新処理
	void Update(DirectInput* input, GamePad* gamePad);

	// 描画処理
	void Draw();

	// ImGui描画
	void DrawImGui();

private:
	// 今フレームの選択移動量
	int PollMove(DirectInput* input, GamePad* gamePad);

	// 決定入力があったか
	bool PollDecide(DirectInput* input, GamePad* gamePad) const;

private:
	// ボタンのコンテナ
	std::vector<Button> buttons_;

	// レイアウト
	Layout layout_ = Layout::Horizontal;

	// オーディオマネージャーポインタ
	TinyEngine::AudioManager* audio_ = nullptr;

	// 選択インデックス
	int selectedIndex_ = 0;

	// スティック入力検知
	bool stickInUse_ = false;

	// 決定したかどうか
	bool isDecided_ = false;
};