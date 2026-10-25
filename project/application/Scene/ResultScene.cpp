#include "ResultScene.h"
#include "SceneManager.h"

using namespace TinyEngine;

void ResultScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;
	commonData_ = ctx_.sceneManager->GetCommonData();

	// クリア時は記録を更新する
	if (commonData_->isClear) {
		commonData_->ReportClear(commonData_->currentStageNo, commonData_->clearTime, commonData_->killCount);
	}

	// メインカメラ
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize();

	// シーンで使うエフェクトの宣言
	ctx_.engineContext->postEffectPipeline->SetEffects({
	    PostEffectType::Scanline,         // 走査線
	    PostEffectType::BarrelDistortion, // 歪み
	    PostEffectType::Glitch,           // グリッチ
	});

	barrelDistortionParam_.strength = 0.05f;
	glitchParam_.intensity = 0.1f;
	scanlineParam_.scanlineCount = 150.0f;
	scanlineParam_.intensity = 0.7f;
	scanlineParam_.speed = 5.0f;

	// オーディオマネージャーの生成&初期化
	audioManager_ = std::make_unique<AudioManager>();
	audioManager_->Initialize();
	audioManager_->LoadWave("ResultBGM", "resources/sounds/bgm/ResultScene.mp3");
	audioManager_->LoadWave("Select", "resources/sounds/se/Select.mp3");
	audioManager_->LoadWave("Decide", "resources/sounds/se/Decide.mp3");
	audioManager_->PlayBGM("ResultBGM");

	// フォントの生成&初期化
	font_ = std::make_unique<Font>();
	font_->Initialize(ctx.engineContext, L"Dela Gothic One", 48, L"体リトライタイトルへ戻る次のステージ");
	titleFont_ = std::make_unique<Font>();
	titleFont_->Initialize(ctx.engineContext, L"Dela Gothic One", 96);

	// リザルトテキスト生成&初期化
	resultText_ = std::make_unique<TextSprite>();
	resultText_->Initialize(ctx.engineContext, titleFont_.get(), commonData_->isClear ? L"CLEAR" : L"GAME OVER");
	resultText_->SetAnchorPoint({0.5f, 0.0f});
	resultText_->SetPosition({640.0f, 80.0f});
	resultText_->SetColor({1.0f, 0.7f, 0.1f, 1.0f});

	// スコア表示インスタンス生成&初期化
	scoreDisplay_ = std::make_unique<ResultScoreDisplay>();
	scoreDisplay_->Initialize(ctx.engineContext, commonData_->isClear, commonData_->clearTime, commonData_->killCount);

	// ボタングループの生成&初期化
	buttons_ = std::make_unique<ButtonGroup>();
	buttons_->Initialize(ButtonGroup::Layout::Horizontal, audioManager_.get());
	buttons_->Add(ctx.engineContext, font_.get(), L"リトライ", {240.0f, 580.0f}, [this]() { ctx_.sceneTransition->ChangeScene(commonData_->currentStageKey, FadeStyle::Logo); });
	buttons_->Add(ctx.engineContext, font_.get(), L"タイトル", {625.0f, 580.0f}, [this]() { ctx_.sceneTransition->ChangeScene("Title", FadeStyle::Logo); });
	buttons_->Add(ctx.engineContext, font_.get(), L"次のステージ", {1030.0f, 580.0f}, [this]() {
		ctx_.sceneTransition->ChangeScene("Stage" + std::to_string(commonData_->currentStageNo + 1), FadeStyle::Logo);
	});
}

void ResultScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

	// 音声更新
	audioManager_->Update();

	// 歪みのパラメータにDeltaTime加算
	glitchParam_.time += deltaTime;
	scanlineParam_.time += deltaTime;

	auto* scanline = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::Scanline);
	if (scanline) {
		scanline->SetScanlineParam(scanlineParam_);
	}

	auto* barrelDistortionParam = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::BarrelDistortion);
	if (barrelDistortionParam) {
		barrelDistortionParam->SetFisheyeParam(barrelDistortionParam_);
	}

	auto* glitch = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::Glitch);
	if (glitch) {
		bool shouldGlitch = commonData_->postEffectSettings.glitchEnabled;
		glitch->SetGlitchTime(glitchParam_.time);
		glitch->SetGlitchIntensity(shouldGlitch ? glitchParam_.intensity : 0.0f);
	}

	// リザルトテキスト更新
	resultText_->Update();

	// スコア表示更新
	scoreDisplay_->Update(deltaTime);

#ifdef USE_IMGUI
	// ボタングループのImGui描画
	ImGui::Begin("Button Editor");
	buttons_->DrawImGui();
	ImGui::End();
#endif

	// ボタングループの更新
	buttons_->Update(ctx_.keyboard, ctx_.gamePad);
}

void ResultScene::Draw() {
	// リザルト
	resultText_->Draw();

	// スコア表示描画
	scoreDisplay_->Draw();

	// ボタングループの描画
	buttons_->Draw();
}

void ResultScene::Finalize() { audioManager_->StopBGM(); }
