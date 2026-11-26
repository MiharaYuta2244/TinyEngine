#include "Game.h"
#include "GameAudio.h"
#include "EffectLibrary.h"
#include "Utility/SettingsStorage.h"
#include "InputBindings.h"
#include "Menu/MenuLayout.h"
#include "Scene/EffectEditorScene.h"
#include "Scene/GamePlayScene.h"
#include "Scene/PauseScene.h"
#include "Scene/ResultScene.h"
#include "Scene/StageSelectScene.h"
#include "Scene/TestScene.h"
#include "Scene/TitleScene.h"

using namespace TinyEngine;

void Game::Initialize() {
	// 基底クラスの初期化処理
	Framework::Initialize();

	// シーンを開始する前に共有の音声システムを初期化し、SEを先読みする
	AudioManager::Lifecycle::Initialize();
	GameAudio::PreloadSE();

	// 起動時時に全エフェクトを読み込み
	EffectLibrary::Get().LoadAll();

	// フェードマネージャーの生成&初期化
	fadeManager_ = std::make_unique<FadeManager>();
	fadeManager_->Initialize(&GetEngineContext());
	fadeManager_->SetStyle(FadeStyle::Logo);

	// シーンマネージャーの生成&初期化
	sceneManager_ = std::make_unique<SceneManager>();
	sceneTransition_ = std::make_unique<SceneTransition>();
	sceneManager_->Initialize(&GetEngineContext(), GetKeyboard(), GetGamePad(), GetDebugCamera(), GetTimeManager(), fadeManager_.get(), sceneTransition_.get());
	sceneTransition_->Initialize(sceneManager_.get(), fadeManager_.get());
	SettingsStorage::Load(sceneManager_->GetCommonData()->postEffectSettings);
	MenuLayoutStore::Get().Load();

	// シーン追加
	sceneManager_->AddScene("Title", std::make_unique<TitleScene>());
	sceneManager_->AddScene("Stage1", std::make_unique<GamePlayScene>(1, "Stage1/"));
	sceneManager_->AddScene("Stage2", std::make_unique<GamePlayScene>(2, "Stage2/"));
	sceneManager_->AddScene("Stage3", std::make_unique<GamePlayScene>(3, "Stage3/"));
	sceneManager_->AddScene("Pause", std::make_unique<PauseScene>());
	sceneManager_->AddScene("Result", std::make_unique<ResultScene>());
	sceneManager_->AddScene("StageSelect", std::make_unique<StageSelectScene>());
	sceneManager_->AddScene("Test", std::make_unique<TestScene>());
	sceneManager_->AddScene("EffectEditor", std::make_unique<EffectEditorScene>());

	// 最初のシーンを指定
	sceneTransition_->ChangeSceneImmediate("Title");
}

void Game::Update() {
	// 基底クラスの更新処理
	Framework::Update();

	// ポストエフェクトのON/OFFを全シーン共通で反映
	ApplyPostEffectSettings();

	// フェード進行とシーン切り替え
	sceneTransition_->Update(GetTimeManager()->GetDeltaTime());

	// シーンマネージャー更新
	sceneManager_->Update();
	// そのフレームのシーン更新後に、再生が終わったSEボイスを回収する
	AudioManager::Lifecycle::Update();
	InputBindings::Get().EndFrame(GetGamePad());
}

void Game::Draw() {
	Framework::BeginRender();

	Framework::SRVManagerPreDraw();

	// シーン描画
	sceneManager_->Draw();

	// フェードスプライトを上書きで描画
	fadeManager_->Draw();

	Framework::EndRender();

	// 描画前処理
	Framework::PreDraw();

	// 描画後処理
	Framework::PostDraw();
}

void Game::Finalize() {
	// シーンマネージャー終了処理
	sceneManager_->Finalize();

	// シーンの終了処理を済ませ、FrameworkがCOMを終了する前に音声を解放する
	AudioManager::Lifecycle::Finalize();

	// 基底クラスの終了処理
	Framework::Finalize();
}

void Game::ApplyPostEffectSettings() {
	const PostEffectSettings& s = sceneManager_->GetCommonData()->postEffectSettings;
	PostEffectPipeline* pipeline = GetEngineContext().postEffectPipeline;

	pipeline->SetEffectEnabled(PostEffectType::Vignette, s.vignetteEnabled);
	pipeline->SetEffectEnabled(PostEffectType::RadialBlur, s.radialBlurEnabled);
	pipeline->SetEffectEnabled(PostEffectType::Glitch, s.glitchEnabled);
	pipeline->SetEffectEnabled(PostEffectType::Scanline, s.scanlineEnabled);
	pipeline->SetEffectEnabled(PostEffectType::BarrelDistortion, s.barrelDistortionEnabled);
	pipeline->SetEffectEnabled(PostEffectType::Smoothing, s.blurEnabled);
	pipeline->SetEffectEnabled(PostEffectType::Gaussian, s.blurEnabled);
	pipeline->SetEffectEnabled(PostEffectType::DeathEffect, s.deathEffectEnabled);
	pipeline->SetEffectEnabled(PostEffectType::DepthOutline, s.outlineEnabled);
	pipeline->SetEffectEnabled(PostEffectType::LuminanceOutline, s.outlineEnabled);
}
