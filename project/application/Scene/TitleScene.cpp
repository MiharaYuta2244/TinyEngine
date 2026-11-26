#include "TitleScene.h"
#include "Game/GameAudio.h"
#include "GameObjects/PlaytestConfig.h"
#include "SceneManager.h"

using namespace TinyEngine;

void TitleScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;
	commonData_ = ctx_.sceneManager->GetCommonData();
	commonData_->killCount = 0;
	commonData_->clearTime = 0.0f;
	commonData_->currentStageNo = 1;
	commonData_->currentStageKey = "Stage1";

	// メニューの生成&初期化
	menu_ = std::make_unique<MenuList>();
	menu_->Initialize(ctx.engineContext, L"Dela Gothic One", 48, true);
	menu_->AddItem(L"ゲーム開始", [this]() { ctx_.sceneTransition->ChangeScene("StageSelect", FadeStyle::Logo); });
	menu_->AddItem(L"オプション", [this]() {
		menuState_ = TitleMenuState::Option;
		optionMenu_->ResetState();
	});
	menu_->AddItem(L"終了", [this]() {
		finishTimer_ = std::make_unique<GameTimer>();
		finishTimer_->Initialize(0.5f);
	});
	menu_->SetStartPos({520.0f, 350.0f});
	menu_->BindLayout("title");

	// ポーズ画面に移動せずに、起動画面から同じレイアウトを編集できるようにする。
	pausePreview_ = std::make_unique<MenuList>();
	pausePreview_->Initialize(ctx.engineContext, L"Dela Gothic One", 30, true);
	pausePreview_->AddItem(L"再開", []() {}, true);
	pausePreview_->AddItem(L"オプション", []() {}, true);
	pausePreview_->AddItem(L"タイトルへ戻る", []() {}, true);
	pausePreview_->BindLayout("pause");
	previewFont_ = std::make_unique<Font>();
	previewFont_->Initialize(ctx.engineContext, L"Dela Gothic One", 20, Font::MakeJapaneseCharSet(true));
	previewHeading_ = std::make_unique<TextSprite>();
	previewHeading_->Initialize(ctx.engineContext, previewFont_.get(), L"ポーズ");

	// オプションの各カテゴリはタイトルとポーズで共通利用する。
	optionMenu_ = std::make_unique<OptionMenu>();
	optionMenu_->Initialize(ctx.engineContext, &commonData_->postEffectSettings, [this]() {
		menuState_ = TitleMenuState::Main;
		menu_->ResetState();
	});

	// タイトルロゴの生成&初期化
	titleLogo_ = std::make_unique<Sprite>();
	titleLogo_->Initialize(ctx.engineContext, "Title_Logo.png");
	titleLogo_->SetPosition(titleLogoPos_);
	titleLogo_->SetEnableShine(true);

	// 背景の生成&初期化
	background_ = std::make_unique<Sprite>();
	background_->Initialize(ctx.engineContext, "white.png");
	background_->SetSize({1280.0f, 720.0f});
	background_->SetColor(backgroundColor_);
	background_->SetEnableVoronoi(true);
	background_->SetVoronoiColor(voronoiColor_);
	background_->SetZDepth(100.0f);

	// シーンで使うエフェクトの宣言
	ctx_.engineContext->postEffectPipeline->SetEffects({
	    PostEffectType::Scanline,         // 走査線
	    PostEffectType::BarrelDistortion, // ブラウン管風
	    PostEffectType::Glitch,           // グリッチ
	});

	barrelDistortionParam_.strength = 0.05f;
	glitchParam_.intensity = 0.1f;
	scanlineParam_.scanlineCount = 150.0f;
	scanlineParam_.intensity = 0.7f;
	scanlineParam_.speed = 5.0f;

	TinyEngine::AudioManager::Get().PlayBGM(GameAudio::BGM::Title);

	// 黒背景生成&初期化
	blackBg_ = std::make_unique<Sprite>();
	blackBg_->Initialize(ctx.engineContext, "white.png");
	blackBg_->SetSize({1280.0f, 720.0f});
	blackBg_->SetColor({0, 0, 0, 1});

	// コントローラー画像生成&初期化
	controllerImage_ = std::make_unique<Sprite>();
	controllerImage_->Initialize(ctx.engineContext, "Controller.png");
	controllerImage_->SetPosition({640.0f, 360.0f});
	controllerImage_->SetAnchorPoint({0.5f, 0.5f});

	// コントローラー推奨テキスト生成&初期化
	recommended_ = std::make_unique<Sprite>();
	recommended_->Initialize(ctx.engineContext, "Recommended.png");
	recommended_->SetPosition({640.0f, 600.0f});
	recommended_->SetAnchorPoint({0.5f, 0.5f});
}

void TitleScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();
	const auto preview = MenuLayoutStore::Get().DrawImGui();
	previewPause_ = preview == "pause";
	if (preview == "title") {
		if (menuState_ != TitleMenuState::Main) {
			menu_->ResetState();
		}

		menuState_ = TitleMenuState::Main;
	} else if (optionMenu_->PreviewLayout(preview)) {
		menuState_ = TitleMenuState::Option;
	}

#if defined(NDEBUG) || defined(_DEBUG)
	// エフェクトエディタシーンへの遷移
	if (menuState_ == TitleMenuState::Main && ctx_.keyboard->KeyTriggered(DIK_F5)) {
		ctx_.sceneTransition->ChangeScene("EffectEditor", FadeStyle::Logo);
	}
#endif

	// 黒背景＆コントローラー画像のフェードアウト処理
	float totalFadeTime = fadeWaitDuration_ + fadeOutDuration_;
	bool isFading = (fadeTimer_ < totalFadeTime);
	bool skippedThisFrame = false;

	// 初回のみフェード処理およびスキップ入力チェックを行う
	if (isRecommended_) {
		isFading = (fadeTimer_ < totalFadeTime);

		if (isFading) {
			fadeTimer_ += deltaTime;
			float alpha = 1.0f;

			// 透明度を下げる
			if (fadeTimer_ > fadeWaitDuration_) {
				float progress = (fadeTimer_ - fadeWaitDuration_) / fadeOutDuration_;
				alpha = std::clamp(1.0f - progress, 0.0f, 1.0f);
			}

			// アルファ値を適用
			blackBg_->SetColor({0.0f, 0.0f, 0.0f, alpha});
			controllerImage_->SetColor({1.0f, 1.0f, 1.0f, alpha});
			recommended_->SetColor({1.0f, 1.0f, 1.0f, alpha});

			// ゲームパッド推奨スキップ
			if (ctx_.keyboard->KeyTriggered(DIK_SPACE) || ctx_.gamePad->GetState().buttons.a) {
				fadeTimer_ = totalFadeTime;
				isFading = false;
				isRecommended_ = false;  // 次回以降表示しないようにフラグを倒す
				skippedThisFrame = true; // このフレームでスキップされたことを記録
			}
		} else {
			// フェード時間が終了したら表示完了とみなす
			isRecommended_ = false;
		}
	}

	// メニューの更新
	if (!isFading && !skippedThisFrame) {
		if (previewPause_) {
			pausePreview_->Update(ctx_.keyboard, ctx_.gamePad, deltaTime);
		} else if (menuState_ == TitleMenuState::Main) {
			menu_->Update(ctx_.keyboard, ctx_.gamePad, deltaTime);
		} else {
			optionMenu_->Update(ctx_.keyboard, ctx_.gamePad, deltaTime);
		}
	}

	// タイトルロゴ更新
	titleLogo_->SetPosition(titleLogoPos_);
	titleLogo_->SetColor(logoColor_);
	titleLogo_->Update();

	// 背景更新
	voronoiTimer_ += deltaTime;
	background_->SetVoronoiParams(voronoiParams_.x, voronoiParams_.y, voronoiParams_.z, voronoiTimer_);
	background_->SetColor(backgroundColor_);
	background_->SetVoronoiColor(voronoiColor_);
	background_->Update();

	// 黒背景更新
	blackBg_->Update();

	// コントローラー画像更新
	controllerImage_->Update();

	// コントローラー推奨テキスト更新
	recommended_->Update();

	// 歪みのパラメータにDeltaTime加算
	glitchParam_.time += deltaTime;
	scanlineParam_.time += deltaTime;

	// ゲーム終了タイマー更新
	if (finishTimer_) {
		finishTimer_->Update(deltaTime);

		if (finishTimer_->IsEnd()) {
			// ゲーム終了
			PostQuitMessage(0);
		}
	}

#ifdef USE_IMGUI
	ImGui::Begin("PostEffect");

	// =======================
	// Scanline
	// =======================
	if (ImGui::CollapsingHeader("Scanline")) {
		ImGui::DragFloat("ScanlineCount", &scanlineParam_.scanlineCount, 1.0f, 1.0f, 2000.0f);
		ImGui::DragFloat("Intensity", &scanlineParam_.intensity, 0.01f, 0.0f, 5.0f);
		ImGui::DragFloat("Speed", &scanlineParam_.speed, 0.01f, 0.0f, 5.0f);
	}

	// =======================
	// Fisheye
	// =======================
	if (ImGui::CollapsingHeader("Fisheye")) {
		ImGui::DragFloat("Strength", &barrelDistortionParam_.strength, 0.01f, 0.0f, 5.0f);
	}

	// =======================
	// Glitch
	// =======================
	if (ImGui::CollapsingHeader("Glitch")) {
		ImGui::DragFloat("Intensity", &glitchParam_.intensity, 0.01f, 0.0f, 5.0f);
	}

	ImGui::End();
#endif

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
		glitch->SetGlitchTime(glitchParam_.time);
		glitch->SetGlitchIntensity(glitchParam_.intensity);
	}
}

void TitleScene::Draw() {
	// 背景描画
	background_->Draw();

	// メニュー描画
	if (previewPause_) {
		pausePreview_->Draw();
		const auto& layout = MenuLayoutStore::Get().Layout("pause");
		previewHeading_->SetPosition(layout.headingPosition);
		previewHeading_->SetScale(layout.headingScale);
		previewHeading_->SetColor(layout.colors.heading);
		previewHeading_->Update();
		previewHeading_->Draw();
	} else if (menuState_ == TitleMenuState::Main) {
		menu_->Draw();
	} else {
		optionMenu_->Draw();
	}

	// タイトルロゴ描画
	titleLogo_->Draw();

	if (isRecommended_) {
		// 黒背景描画
		blackBg_->Draw();

		// コントローラー画像描画
		controllerImage_->Draw();

		// コントローラー推奨テキスト描画
		recommended_->Draw();
	}
}

void TitleScene::Finalize() { TinyEngine::AudioManager::Get().StopBGM(); }
