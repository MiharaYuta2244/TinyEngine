#pragma once
#include "BaseScene.h"
#include "Menu/MenuList.h"
#include "AudioManager.h"
#include "GameTimer.h"

class TitleScene : public BaseScene {
public:
	void Initialize(const SceneContext& ctx) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// メイン/オプションのメニュー状態
	enum class TitleMenuState { Main, Option };
	TitleMenuState menuState_ = TitleMenuState::Main;

	// メニュー
	std::unique_ptr<MenuList> menu_;
	std::unique_ptr<MenuList> optionMenu_;

	// タイトルロゴ
	std::unique_ptr<TinyEngine::Sprite> titleLogo_;
	Vector2 titleLogoPos_ = {70.0f, 100.0f};        // ロゴの座標
	Vector4 logoColor_ = {1.0f, 0.7f, 0.1f, 1.0f}; // 色

	// 背景
	std::unique_ptr<TinyEngine::Sprite> background_;
	Vector3 voronoiParams_ = {5.0f, 2.0f, 1.0f};         // ボロノイノイズのパラメータ
	float voronoiTimer_ = 0.0f;                          // ボロノイノイズ用タイマー
	Vector4 voronoiColor_ = {0.0f, 0.2f, 0.2f, 1.0f};    // ボロノイノイズの色
	Vector4 backgroundColor_ = {0.0f, 0.0f, 0.0f, 1.0f}; // 背景色

	// ポストエフェクトパラメータ
	ScanlineParam scanlineParam_;                 // 走査線
	BarrelDistortionParam barrelDistortionParam_; // 魚眼
	GlitchParam glitchParam_;                     // グリッチ

	// オーディオマネージャーインスタンス
	std::unique_ptr<TinyEngine::AudioManager> audioManager_;

	// ゲーム終了までのタイマー
	std::unique_ptr<GameTimer> finishTimer_;

	// ゲーム機同時の黒背景
	std::unique_ptr<TinyEngine::Sprite> blackBg_;

	// コントローラー画像
	std::unique_ptr<TinyEngine::Sprite> controllerImage_;

	// コントローラー推奨テキスト
	std::unique_ptr<TinyEngine::Sprite> recommended_;

	float fadeTimer_ = 0.0f;              // フェード用タイマー
	const float fadeWaitDuration_ = 2.0f; // 表示を維持する時間
	const float fadeOutDuration_ = 2.0f;  // フェードアウトにかける時間

	// 一度表示したら以降はタイトル画面で表示しない為のフラグ
	static inline bool isReccomended_ = true;
};
