#include "StageSelectScene.h"
#include "SceneManager.h"
#include <algorithm>
#include <cmath>
#include <format>
#include <numbers>

using namespace TinyEngine;

namespace {
const std::array<const char*, 3> kStageModels = {"Cube.obj", "Cube.obj", "Cube.obj"};
const std::array<const char*, 3> kScreenshots = {"Stage1_Screenshot.png", "Stage2_Screenshot.png", "Stage3_Screenshot.png"};
const char* kDotTexture = "Circle.png";    // 点線用
const char* kMarkerTexture = "Circle.png"; // マーカー用

const Vector2 kScreenshotPos = {60.0f, 50.0f};
const Vector2 kScreenshotSize = {420.0f, 236.0f};
const float kFramePadding = 8.0f;

const Vector4 kMarkerClearedColor = {1.0f, 0.85f, 0.2f, 1.0f};    // クリアの色
const Vector4 kMarkerNotClearedColor = {0.4f, 0.4f, 0.45f, 1.0f}; // 未クリアの色
const Vector4 kMarkerLockedColor = {0.15f, 0.15f, 0.18f, 1.0f};   // 未開放の色

const float kDotSpacing = 1.5f;                                // 点線の間隔
const float kConnectionDotHeight = 0.10f;                      // 点線の高さ
const Vector3 kConnectionDotScale = {0.2f, 0.2f, 1.0f};        // 点線1つの大きさ
const Vector4 kConnectionDotColor = {0.7f, 0.7f, 0.75f, 0.8f}; // 点線の色
const int kRingDotCount = 16;
const float kPi = std::numbers::pi_v<float>;
} // namespace

void StageSelectScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;
	commonData_ = ctx_.sceneManager->GetCommonData();

	// 前回作成したデカールへのポインタを破棄
	ringDots_.clear();
	connectionDots_.clear();
	markers_ = {};

	selected_ = std::clamp(commonData_->currentStageNo - 1, 0, kStageCount - 1);
	isDecided_ = false;
	stickInUse_ = false;
	modelTime_ = 0.0f;

	// カメラの生成&初期化
	camera_ = std::make_unique<Camera>();
	camera_->Initialize();

	// Object3dはInitialize時にデフォルトカメラを掴むので先に設定する
	ctx_.engineContext->object3dCommon->SetDefaultCamera(camera_.get());
	currentAngle_ = GetStageAngle(selected_);
	UpdateCamera(0.0f);

	// ライトの初期化
	DirectionalLight dirLight;
	dirLight.color = {1, 1, 1, 1};
	dirLight.intensity = 3.0f;
	dirLight.direction = {0.5f, 0.5f, -0.5f};
	ctx_.engineContext->object3dCommon->SetDirectionalLightParam(dirLight);
	PointLight pointLight = ctx_.engineContext->object3dCommon->GetPointLight();
	pointLight.intensity = 0.0f;
	ctx_.engineContext->object3dCommon->SetPointLightParam(pointLight);

	// デカールの生成&初期化
	decalManager_ = std::make_unique<DecalManager>();
	decalManager_->Initialize(ctx_.engineContext);
	decalManager_->SetCamera(camera_.get());

	// ステージモデルの生成&初期化
	for (int i = 0; i < kStageCount; ++i) {
		miniatures_[i] = std::make_unique<StageMiniature>();
		miniatures_[i]->Initialize(ctx_.engineContext, "Stage" + std::to_string(i + 1) + "/");
	}

	const Vector3 flat = {kPi / 2.0f, 0.0f, 0.0f}; // 地面に水平

	// ステージ間をつなぐ点線
	CreateConnectionLine();

	// 選択リング
	for (int k = 0; k < kRingDotCount; ++k) {
		ringDots_.push_back(decalManager_->AddDecal(kDotTexture, {0.0f, 0.11f, 0.0f}, flat, {0.12f, 0.12f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}));
	}

	// マーカー
	for (int i = 0; i < kStageCount; ++i) {
		markers_[i] = decalManager_->AddDecal(kMarkerTexture, GetArcPos(GetStageAngle(i), arcRadius_, 0.12f), flat, {0.8f, 0.8f, 1.0f}, kMarkerNotClearedColor);
	}

	ApplyMarkerColors();

	// 背景の生成&初期化
	background_ = std::make_unique<Sprite>();
	background_->Initialize(ctx_.engineContext, "white.png");
	background_->SetSize({1280.0f, 720.0f});
	background_->SetColor({0.1f, 0.1f, 0.2f, 1.0f});
	background_->SetZDepth(100.0f); // 3Dの奥に回す

	// ステージのスクショ生成&初期化
	screenshotFrame_ = std::make_unique<Sprite>();
	screenshotFrame_->Initialize(ctx_.engineContext, "white.png");
	screenshotFrame_->SetPosition({kScreenshotPos.x - kFramePadding, kScreenshotPos.y - kFramePadding});
	screenshotFrame_->SetSize({kScreenshotSize.x + kFramePadding * 2.0f, kScreenshotSize.y + kFramePadding * 2.0f});
	screenshotFrame_->SetColor({0.9f, 0.9f, 0.9f, 1.0f});

	for (int i = 0; i < kStageCount; ++i) {
		screenshots_[i] = std::make_unique<Sprite>();
		screenshots_[i]->Initialize(ctx_.engineContext, kScreenshots[i]);
		screenshots_[i]->SetPosition(kScreenshotPos);
		screenshots_[i]->SetSize(kScreenshotSize);
	}
	screenshotAlpha_ = 1.0f;

	// フォントの生成&初期化
	font_ = std::make_unique<Font>();
	std::wstring chars = Font::MakeJapaneseCharSet() + L"数中解放";
	font_->Initialize(ctx_.engineContext, L"Dela Gothic One", 48, chars);

	const float textY = kScreenshotPos.y + kScreenshotSize.y + 40.0f;
	clearTimeText_ = std::make_unique<TextSprite>();
	clearTimeText_->Initialize(ctx_.engineContext, font_.get(), L"Clear Time  --:--");
	clearTimeText_->SetPosition({kScreenshotPos.x, textY});
	clearTimeText_->SetScale(0.7f);

	killCountText_ = std::make_unique<TextSprite>();
	killCountText_->Initialize(ctx_.engineContext, font_.get(), L"Kill Count  --");
	killCountText_->SetPosition({kScreenshotPos.x, textY + 70.0f});
	killCountText_->SetScale(0.7f);

	// オーディオマネージャーの生成&初期化
	audioManager_ = std::make_unique<AudioManager>();
	audioManager_->Initialize();
	audioManager_->LoadWave("Select", "resources/sounds/se/Select.mp3");
	audioManager_->LoadWave("Decide", "resources/sounds/se/Decide.mp3");
	audioManager_->LoadWave("Error", "resources/sounds/se/Error.mp3");

	// ポストエフェクトの設定
	ctx_.engineContext->postEffectPipeline->SetEffects({
	    PostEffectType::Scanline,
	    PostEffectType::BarrelDistortion,
	    PostEffectType::Glitch,
	});

	// ポストエフェクトパラメータ初期化
	barrelDistortionParam_.strength = 0.05f;
	glitchParam_.intensity = 0.1f;
	scanlineParam_.scanlineCount = 150.0f;
	scanlineParam_.intensity = 0.7f;
	scanlineParam_.speed = 5.0f;

	// 決定時アニメーションパラメータ初期化
	isDecideAnimating_ = false;
	decideSpin_ = 0.0f;
	decideJump_ = 0.0f;
	decideSpinAnim_.Reset();
	decideJumpUpAnim_.Reset();
	decideJumpDownAnim_.Reset();
}

void StageSelectScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

#if defined(NDEBUG) || defined(_DEBUG)
	// F6で全ステージ開放のON/OFF
	if (ctx_.keyboard->KeyTriggered(DIK_F6)) {
		commonData_->unlockAllStages = !commonData_->unlockAllStages;
	}
#endif

	// マーカーの色を毎フレーム反映
	ApplyMarkerColors();

	// オーディオマネージャーの更新
	audioManager_->Update();

	// 入力処理
	HandleInput();

	// カメラの更新
	UpdateCamera(deltaTime);

	// モデルの更新
	UpdateModels(deltaTime);

	// デカールの更新
	UpdateRing(deltaTime);
	decalManager_->SetCamera(camera_.get());
	decalManager_->Update(deltaTime);

	// スクショ等の情報を更新
	UpdateInfo(deltaTime);

	// 背景更新
	background_->Update();

	// スクショ背景の更新
	screenshotFrame_->Update();

#ifdef USE_IMGUI
	ImGui::Begin("StageSelect");
	ImGui::DragFloat("Camera Distance", &cameraDistance_, 0.1f);
	ImGui::DragFloat("Camera Height", &cameraHeight_, 0.1f);
	ImGui::DragFloat("Camera Pitch", &cameraPitch_, 0.01f);
	ImGui::DragFloat("Camera Lateral", &cameraLateral_, 0.1f);
	ImGui::DragFloat("Move Duration", &cameraMoveDuration_, 0.01f, 0.1f, 3.0f);
	ImGui::DragFloat("Model Scale", &modelScale_, 0.05f);
	ImGui::DragFloat("Model Height", &modelHeight_, 0.05f);
	ImGui::DragFloat("Rotate Speed", &rotateSpeed_, 0.01f);
	ImGui::DragFloat("Bob Amplitude", &bobAmplitude_, 0.01f);
	ImGui::DragFloat("Ring Radius", &ringRadius_, 0.05f);
	ImGui::End();
#endif

	// ポストエフェクト
	glitchParam_.time += deltaTime;
	scanlineParam_.time += deltaTime;

	if (auto* scanline = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::Scanline)) {
		scanline->SetScanlineParam(scanlineParam_);
	}
	if (auto* barrel = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::BarrelDistortion)) {
		barrel->SetFisheyeParam(barrelDistortionParam_);
	}
	if (auto* glitch = ctx_.engineContext->postEffectPipeline->GetPass(PostEffectType::Glitch)) {
		glitch->SetGlitchTime(glitchParam_.time);
		glitch->SetGlitchIntensity(glitchParam_.intensity);
	}
}

void StageSelectScene::Draw() {
	// 背景
	background_->Draw();

	// ミニチュア描画
	for (auto& miniature : miniatures_) {
		miniature->Draw();
	}

	// デカール描画
	decalManager_->Draw();

	// 透明のミニチュア描画
	for (auto& miniature : miniatures_) {
		miniature->DrawTransparent(); // ガラス
	}

	// マーカー・点線・選択リング
	decalManager_->Draw();

	// 左上の情報
	screenshotFrame_->Draw();
	screenshots_[selected_]->Draw();
	clearTimeText_->Draw();
	killCountText_->Draw();
}

void StageSelectScene::Finalize() {
	// デカールへのポインタ破棄
	ringDots_.clear();
	connectionDots_.clear();
	markers_ = {};

	// ミニチュア解放
	for (auto& miniature : miniatures_) {
		miniature.reset();
	}

	// デフォルトカメラを共通のカメラに戻す
	ctx_.engineContext->object3dCommon->SetDefaultCamera(ctx_.currentCamera);
}

void StageSelectScene::HandleInput() {
	DirectInput* input = ctx_.keyboard;

	bool left = input->KeyTriggered(DIK_A) || input->KeyTriggered(DIK_LEFT);
	bool right = input->KeyTriggered(DIK_D) || input->KeyTriggered(DIK_RIGHT);
	bool decide = input->KeyTriggered(DIK_SPACE);
	bool back = input->KeyTriggered(DIK_ESCAPE);

	if (ctx_.gamePad && ctx_.gamePad->GetState().connected) {
		const auto& pad = ctx_.gamePad->GetState();
		float lx = pad.axes.lx;

		if (!stickInUse_) {
			if (lx > 0.9f) {
				right = true;
				stickInUse_ = true;
			} else if (lx < -0.9f) {
				left = true;
				stickInUse_ = true;
			}
		}
		if (std::abs(lx) < 0.3f) {
			stickInUse_ = false;
		}

		left |= pad.buttonsPressed.dpadLeft;
		right |= pad.buttonsPressed.dpadRight;
		decide |= pad.buttonsPressed.a;
		back |= pad.buttonsPressed.b;
	}

	if (isDecided_) {
		return;
	}

	if (left) {
		ChangeSelection(-1);
	}
	if (right) {
		ChangeSelection(+1);
	}

	if (decide) {
		// 未開放ステージには入れない
		if (!commonData_->IsStageUnlocked(selected_ + 1)) {
			audioManager_->PlaySE("Error", 0.5f);
			return;
		}

		audioManager_->PlaySE("Decide", 0.5f);
		StartDecideAnimation();
		isDecided_ = true;
	} else if (back) {
		ctx_.sceneTransition->ChangeScene("Title", FadeStyle::Logo);
		isDecided_ = true;
	}
}

void StageSelectScene::ChangeSelection(int delta) {
	int next = std::clamp(selected_ + delta, 0, kStageCount - 1);
	if (next == selected_) {
		return;
	}
	selected_ = next;

	// 現在のカメラ角度から次のステージの角度へ
	cameraAngleAnim_.Start(currentAngle_, GetStageAngle(selected_), cameraMoveDuration_, EaseType::EASEINOUTCUBIC);

	// スクショをフェードインさせる
	screenshotAlphaAnim_.Start(0.0f, 1.0f, 0.3f, EaseType::EASEOUTCUBIC);
	screenshotAlpha_ = 0.0f;

	// SE再生
	audioManager_->PlaySE("Select", 0.5f);
}

void StageSelectScene::UpdateCamera(float deltaTime) {
	// アクティブな間だけ値が書き込まれる
	cameraAngleAnim_.Update(deltaTime, currentAngle_);

	Vector3 dir = {std::sin(currentAngle_), 0.0f, std::cos(currentAngle_)};
	Vector3 right = {dir.z, 0.0f, -dir.x};

	// 弧の内側に立って外向きに見る。横にずらしてモデルを画面右寄りにする
	float r = arcRadius_ - cameraDistance_;
	Vector3 pos = {dir.x * r - right.x * cameraLateral_, cameraHeight_, dir.z * r - right.z * cameraLateral_};

	camera_->SetTranslation(pos);
	camera_->SetRotate({cameraPitch_, currentAngle_, 0.0f});
}

void StageSelectScene::UpdateModels(float deltaTime) {
	modelTime_ += deltaTime;
	UpdateDecideAnimation(deltaTime);

	for (int i = 0; i < kStageCount; ++i) {
		float bob = std::sin(modelTime_ * bobSpeed_ + static_cast<float>(i) * 1.3f) * bobAmplitude_;
		float yaw = modelTime_ * rotateSpeed_ + static_cast<float>(i) * 0.8f;
		Vector3 pos = GetArcPos(GetStageAngle(i), arcRadius_, modelHeight_ + bob);

		// 決定演出は選択中のミニチュアだけに加算
		if (i == selected_) {
			yaw += decideSpin_;
			pos.y += decideJump_;
		}

		// 未選択のミニチュアは暗くする
		float c = (i == selected_) ? 1.0f : 0.45f;

		// ミニチュア更新
		miniatures_[i]->Update(pos, yaw, miniScale_, c);
	}
}

void StageSelectScene::UpdateRing(float deltaTime) {
	ringRotation_ += ringSpeed_ * deltaTime;

	Vector3 center = GetArcPos(GetStageAngle(selected_), arcRadius_, 0.11f);
	for (int k = 0; k < kRingDotCount; ++k) {
		float theta = ringRotation_ + static_cast<float>(k) * (2.0f * kPi / static_cast<float>(kRingDotCount));
		ringDots_[k]->transform.translate = {center.x + std::cos(theta) * ringRadius_, 0.11f, center.z + std::sin(theta) * ringRadius_};
	}
}

void StageSelectScene::UpdateInfo(float deltaTime) {
	const bool unlocked = commonData_->IsStageUnlocked(selected_ + 1);

	// スクショ（未開放は暗く）
	screenshotAlphaAnim_.Update(deltaTime, screenshotAlpha_);
	float shade = unlocked ? 1.0f : 0.25f;
	screenshots_[selected_]->SetColor({shade, shade, shade, screenshotAlpha_});
	screenshots_[selected_]->Update();

	// 最速タイム・最大撃破数
	if (!unlocked) {
		clearTimeText_->SetText(L"ロック中");
		killCountText_->SetText(L"ステージクリアで解放");
	} else {
		const StageRecord& rec = commonData_->stageRecords[selected_];
		if (rec.cleared) {
			int minutes = std::min(static_cast<int>(rec.bestTime) / 60, 99);
			int seconds = static_cast<int>(rec.bestTime) % 60;
			clearTimeText_->SetText(std::format(L"クリアタイム  {:02}:{:02}", minutes, seconds));
			killCountText_->SetText(std::format(L"キル数  {}", rec.maxKillCount));
		} else {
			clearTimeText_->SetText(L"クリアタイム  --:--");
			killCountText_->SetText(L"キル数  --");
		}
	}

	clearTimeText_->Update();
	killCountText_->Update();
}

void StageSelectScene::ApplyMarkerColors() {
	for (int i = 0; i < kStageCount; ++i) {
		if (!commonData_->IsStageUnlocked(i + 1)) {
			markers_[i]->color = kMarkerLockedColor; // 未開放
		} else if (commonData_->stageRecords[i].cleared) {
			markers_[i]->color = kMarkerClearedColor; // クリア
		} else {
			markers_[i]->color = kMarkerNotClearedColor; // 未クリア
		}
	}
}

float StageSelectScene::GetStageAngle(int index) const {
	// 中央のステージが角度0、左右にarcStepRad_ずつ
	return (static_cast<float>(index) - static_cast<float>(kStageCount - 1) * 0.5f) * arcStepRad_;
}

Vector3 StageSelectScene::GetArcPos(float angle, float radius, float y) const { return {std::sin(angle) * radius, y, std::cos(angle) * radius}; }

void StageSelectScene::CreateConnectionLine() {
	// 点線デカールを地面に水平に置くための回転
	const Vector3 flatRotate = {kPi / 2.0f, 0.0f, 0.0f};

	// ステージ間ごとに点線を生成する
	for (int i = 0; i < kStageCount - 1; ++i) {
		// この区間の開始角度
		float startAngle = GetStageAngle(i);

		// この区間の終了角度
		float endAngle = GetStageAngle(i + 1);

		// 弧の長さ = 半径 × 角度差
		float arcLength = arcRadius_ * (endAngle - startAngle);

		// 弧の長さを点線間隔で割り、必要な点の数を決める
		int segmentCount = std::max(1, static_cast<int>(std::round(arcLength / kDotSpacing)));

		// 区間を指定した数に分割し、境界点にドットを置く
		for (int s = 0; s <= segmentCount; ++s) {
			// 始点から終点の進行度
			float progress = static_cast<float>(s) / segmentCount;

			// 進行度に応じて角度を線形補間
			float angle = startAngle + (endAngle - startAngle) * progress;

			// 弧上の座標を計算
			Vector3 pos = GetArcPos(angle, arcRadius_, kConnectionDotHeight);

			// 点線デカールを追加
			connectionDots_.push_back(decalManager_->AddDecal(kDotTexture, pos, flatRotate, kConnectionDotScale, kConnectionDotColor));
		}
	}
}

void StageSelectScene::StartDecideAnimation() {
	decideSpinAnim_.Start(0.0f, 2.0f * kPi, decideSpinDuration_, EaseType::EASEINOUTCUBIC);
	decideJumpUpAnim_.Start(0.0f, decideJumpHeight_, decideJumpUpDuration_, EaseType::EASEOUTQUAD);
	isDecideAnimating_ = true;
}

void StageSelectScene::UpdateDecideAnimation(float deltaTime) {
	// 回転
	decideSpinAnim_.Update(deltaTime, decideSpin_);

	// ジャンプ上昇
	bool wasUp = decideJumpUpAnim_.GetIsActive();
	decideJumpUpAnim_.Update(deltaTime, decideJump_);

	// 落下開始
	if (wasUp && !decideJumpUpAnim_.GetIsActive()) {
		decideJumpDownAnim_.Start(decideJumpHeight_, 0.0f, decideJumpDownDuration_, EaseType::EASEINQUAD);
	}

	// 落下更新
	decideJumpDownAnim_.Update(deltaTime, decideJump_);

	// アニメーションがすべて終わったらシーン遷移
	if (isDecideAnimating_ && !decideSpinAnim_.GetIsActive() && !decideJumpUpAnim_.GetIsActive() && !decideJumpDownAnim_.GetIsActive()) {
		isDecideAnimating_ = false;
		commonData_->currentStageNo = selected_ + 1;
		commonData_->currentStageKey = "Stage" + std::to_string(selected_ + 1);
		ctx_.sceneTransition->ChangeScene(commonData_->currentStageKey, FadeStyle::Logo);
	}
}