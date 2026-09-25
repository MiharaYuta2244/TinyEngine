#include "GamePlayScene.h"
#include "GameObjects/Effect/EffectGenerator.h"
#include "SceneManager.h"

using namespace TinyEngine;

void GamePlayScene::Initialize(const SceneContext& ctx) {
	ctx_ = ctx;
	commonData_ = ctx_.sceneManager->GetCommonData();

	// 共通データをリセット
	commonData_->killCount = 0;
	commonData_->clearTime = 0.0f;
	commonData_->currentStageNo = stageNo_;
	commonData_->currentStageKey = "Stage" + std::to_string(stageNo_);

	// デバッグカメラ
	debugCamera_ = std::make_unique<Camera>();
	debugCamera_->Initialize();

	// プレイヤーの生成
	player_ = std::make_unique<Player>();

	// ライトの設定
	DirectionalLight dirLight;
	dirLight.color = {1, 1, 1, 1};
	dirLight.intensity = 3.0f;
	dirLight.direction = {0.5f, 0.5f, -0.5f};
	ctx.engineContext->object3dCommon->SetDirectionalLightParam(dirLight);

	// カメラの生成&初期化
	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize();

	// カメラの設定
	ctx_.currentCamera = mainCamera_.get();
	ctx_.engineContext->object3dCommon->SetDefaultCamera(ctx_.currentCamera);
	ctx_.engineContext->particleCommon->SetDefaultCamera(ctx_.currentCamera);

	// デカール管理インスタンス
	decalManager_ = std::make_unique<DecalManager>();
	decalManager_->Initialize(ctx.engineContext);

	// プレイヤーの初期化
	player_->Initialize(ctx_.engineContext, decalManager_.get());

	cameraPosY_ = -20.0f;
	ctx_.currentCamera->SetTranslation({0.0f, 60.0f, 0.0f});

	// カメラのパラメータ初期化
	InitializeGameSceneCamera();

	// 敵の生成&初期化
	enemyManager_ = std::make_unique<EnemyManager>();
	enemyManager_->Initialize(ctx_.engineContext, decalManager_.get(), stagePath_);

	// 敵の弾管理インスタンス生成
	enemyBulletManager_ = std::make_unique<EnemyBulletManager>();

	// 敵の爆弾管理インスタンス生成
	enemyBombManager_ = std::make_unique<EnemyBombManager>();

	// フラッシュエフェクトの生成&初期化
	flashEffect_ = std::make_unique<FlashEffect>();
	flashEffect_->Initialize(ctx_.engineContext);

	// レターボックスの生成&初期化
	letterBox_ = std::make_unique<LetterBox>();
	letterBox_->Initialize(ctx_.engineContext);

	// ステージ管理インスタンスの生成&初期化
	stage_ = std::make_unique<Stage>();
	stage_->Initialize(ctx, decalManager_.get(), stagePath_);

	// ゴールの座標を取得
	Vector3 goalPos = stage_->GetGoal()->GetTransform().translate;

	// カメラのpivotを求める
	Vector3 playerPos = player_->GetPosition();
	Vector3 playerRot = player_->GetRotation();
	Vector3 forward = {std::sin(playerRot.y), 0.0f, std::cos(playerRot.y)};
	introStartPivot_ = {playerPos.x + forward.x * offsetDistance_, cameraPosYAnim_, playerPos.z + forward.z * offsetDistance_};
	introGoalPivot_ = {goalPos.x + forward.x * offsetDistance_, cameraPosYAnim_, goalPos.z + forward.z * offsetDistance_};

	// フェード待ち状態に設定
	introPhase_ = CameraAnimState::WaitingForFade;
	isIntroPlaying_ = true;

	// シーン遷移要求制御変数
	isTransitionRequested_ = false;

	// ポストエフェクト管理インスタンス生成&初期化
	postEffectController_ = std::make_unique<GamePlayPostEffectController>();
	postEffectController_->Initialize(ctx_);

	// プレイヤー死亡時カメラ演出用インスタンス生成
	cameraZoomController_ = std::make_unique<CameraDeathZoomController>();

	// シーンエディターの生成&初期化
	sceneEditor_ = std::make_unique<SceneEditor>();
	sceneEditor_->Initialize();

	// 当たり判定管理インスタンス生成
	collisionManager_ = std::make_unique<CollisionManager>();

	// 操作方法UI
	controlUI_ = std::make_unique<ControlUI>();
	controlUI_->Initialize(ctx.engineContext, decalManager_.get());

	// ステージ1の時のみ操作説明用のUIを表示
	if (stagePath_ == "Stage1/") {
		controlUI_->AddAttackUIDecal({2, 2, 2});
		controlUI_->AddHoldUIDecal({2, 2, 2});
	}

	// オーディオマネージャーの生成&初期化
	audioManager_ = std::make_unique<AudioManager>();
	audioManager_->Initialize();
	audioManager_->LoadWave("GameSceneBGM", "resources/sounds/bgm/GameScene.mp3");
	audioManager_->PlayBGM("GameSceneBGM");

	// フォント生成&初期化
	font_ = std::make_unique<TinyEngine::Font>();
	font_->Initialize(ctx.engineContext, L"Dela Gothic One", 32);

	// テキスト生成&初期化
	textSprite_ = std::make_unique<TinyEngine::TextSprite>();
	textSprite_->Initialize(ctx.engineContext, font_.get(), L"HP: 100");
	textSprite_->SetPosition({50.0f, 50.0f});
}

void GamePlayScene::Update() {
	float deltaTime = ctx_.timeManager->GetDeltaTime();

	// 音声更新
	audioManager_->Update();

	// カメラの動きだけを進める
	if (isIntroPlaying_) {
		// カメラ演出中にも動かしたい処理
		UpdateCameraIntro(deltaTime);
		stage_->Update(deltaTime, player_->GetPosition(), ctx_.currentCamera);
		player_->PostUpdate();
		enemyManager_->PostUpdate();
		decalManager_->SetCamera(ctx_.currentCamera);
		decalManager_->Update();
		postEffectController_->Update(deltaTime, player_->GetCurrentHP(), player_->IsDead());

		// カメラ演出をスキップ
		if (ctx_.keyboard->KeyDown(DIK_SPACE) || ctx_.gamePad->GetState().buttons.a) {
			isIntroPlaying_ = false;
		}

		return;
	}

	// ポーズ画面
	if (ctx_.keyboard->KeyTriggered(DIK_TAB) || ctx_.gamePad->GetState().buttonsPressed.start) {
		RequestScenePush("Pause");
		return;
	}

	// ゴールしていなければクリアタイムを加算
	if (!stage_->GetGoal()->GetIsGoal() && !player_->IsDead()) {
		commonData_->clearTime += deltaTime;
	} else {
		// 敵をすべて殺す
		enemyManager_->AllDead();
	}

	// ポストエフェクト更新
	postEffectController_->Update(deltaTime, player_->GetCurrentHP(), player_->IsDead());

	// プレイヤーの更新処理
	player_->Update(deltaTime, ctx_.keyboard, ctx_.gamePad, enemyManager_.get());

	// 敵の更新処理
	enemyManager_->Update(deltaTime, player_.get(), enemyBulletManager_.get(), stage_->GetWallManager(), stage_->GetDoorManager(), stage_->GetGlassManager(), enemyBombManager_.get());

	// 敵の弾の更新処理
	enemyBulletManager_->Update(deltaTime);

	// 敵の爆弾の更新処理
	enemyBombManager_->Update(deltaTime);

	// ステージオブジェクトの更新
	stage_->Update(deltaTime, player_->GetPosition(), ctx_.currentCamera);

	// 当たり判定
	collisionManager_->CheckCollisions(
	    player_.get(), enemyManager_.get(), enemyBulletManager_.get(), enemyBombManager_.get(), stage_.get(), ctx_.currentCamera, commonData_,
	    [this](const Vector3& pos) { GenerateEnemyDeathEffect(pos); }, postEffectController_->GetGlitchTimer(), postEffectController_->GetDamageBlurTimer());

	// 押し戻し完了後の最終的な座標で、描画更新&AABB更新
	player_->PostUpdate();
	enemyManager_->PostUpdate();

	// ゴールしていたらフラッシュ開始
	if (stage_->GetGoal()->GetIsGoal()) {
		flashEffect_->Trigger();
	}

	// フラッシュの演出が終わったらレターボックスの出現
	if (flashEffect_->Finish()) {
		letterBox_->Trigger();

		// 死亡演出の開始処理
		postEffectController_->StartDeathAnimation();
	}

	// レターボックスの演出が終わったらシーン遷移
	if (!letterBox_->GetIsActive() && !postEffectController_->GetIsDeathAnimPlaying()) {
		if (!isTransitionRequested_) {
			RequestSceneChange("Result");
			isTransitionRequested_ = true;
		}

		// クリアしたかどうかを記録
		commonData_->isClear = true;
	}

	// プレイヤーが死亡したらシーン遷移
	if (player_->IsDead()) {
		// カメラ演出開始
		Vector3 stageCenterPos = {
		    (introStartPivot_.x + introGoalPivot_.x) * 0.5f,
		    0.0f,
		    (introStartPivot_.z + introGoalPivot_.z) * 0.5f,
		};
		cameraZoomController_->Start(ctx_.currentCamera->GetTranslation(), ctx_.currentCamera->GetEuler(), stageCenterPos);

		// カメラ演出更新
		Vector3 nextPos = cameraZoomController_->Update(deltaTime);
		ctx_.currentCamera->SetTranslation(nextPos);
		ctx_.currentCamera->SetEuler(cameraZoomController_->GetRotation());

		if (!isTransitionRequested_ && cameraZoomController_->GetIsFinished()) {
			RequestSceneChange("Result");
			isTransitionRequested_ = true;

			// クリアしたかどうかを記録
			commonData_->isClear = false;
		}
	}

	// ステージオブジェクト管理インスタンスImGui
	stage_->DrawImGui();

	// 敵の管理インスタンスImGui
	enemyManager_->DrawImGui();

	// 操作方法UIの更新&ImGui
	controlUI_->Update();
	controlUI_->DrawImGui();

	// プレイヤーのImGui
	player_->DrawImGui();

	// 毎フレームオブジェクトリストを生成してシーンエディターに渡す
	std::vector<IGameObject*> editObjects;
	editObjects.push_back(player_.get());
	editObjects.push_back(stage_->GetGoal());
	for (auto& enemy : enemyManager_->GetEnemies()) {
		editObjects.push_back(enemy.get());
	}
	for (auto& wall : stage_->GetWallManager()->GetObjects()) {
		editObjects.push_back(wall.get());
	}
	for (auto& door : stage_->GetDoorManager()->GetObjects()) {
		editObjects.push_back(door.get());
	}
	for (auto& glass : stage_->GetGlassManager()->GetObjects()) {
		editObjects.push_back(glass.get());
	}
	for (auto& cage : stage_->GetCageManager()->GetObjects()) {
		editObjects.push_back(cage.get());
	}
	for (auto& healArea : stage_->GetHealAreaManager()->GetObjects()) {
		editObjects.push_back(healArea.get());
	}

	// シーンエディターの更新
	sceneEditor_->Update(ctx_, editObjects, player_.get(), enemyManager_.get(), cameraZoomController_.get(), cameraPosY_, isDebugCameraActive_, ctx_.currentCamera->GetPivot());

	// パーティクルの更新
	for (auto& particle : enemyDeathEffect_) {
		particle->Update();
	}

	std::erase_if(enemyDeathEffect_, [this](const std::unique_ptr<TinyEngine::Particle>& p) {
		bool isFinished = p->IsFinished();
		if (isFinished) {
			// パーティクルが終了して削除される瞬間だけカメラを揺らす
			ctx_.currentCamera->StartShake(0.2f, 0.3f);
		}
		return isFinished;
	});

	// カメラの追従
	if (!isDebugCameraActive_) {
		ctx_.currentCamera->UpdateFollow(player_->GetPosition(), player_->GetRotation(), offsetDistance_, cameraPosY_, cameraAngle_, tiltSpeed_, deltaTime);
	}

	// プレイヤーが敵を掴んだらシェイク
	if (player_->GetIsGrabTriggerd()) {
		ctx_.currentCamera->StartShake(0.2f, 0.2f);
		player_->SetIsGrabTriggerd(false);
	}

	// カメラのシェイク更新
	ctx_.currentCamera->ShakeCamera(deltaTime, shakePower_);

	// カメラの更新
	ctx_.currentCamera->Update(*ctx_.keyboard, *ctx_.gamePad);

	// フラッシュエフェクト更新
	flashEffect_->Update(deltaTime);

	// レターボックス更新
	letterBox_->Update(deltaTime);

	// 血痕管理インスタンス更新
	decalManager_->SetCamera(ctx_.currentCamera);
	decalManager_->Update();

	// テキスト更新
	textSprite_->SetText(L"HP: " + std::to_wstring(100));
	textSprite_->Update();

#ifdef USE_IMGUI
	ImGui::Begin("Camera");
	ImGui::DragFloat("angle", &cameraAngle_, 0.01f);
	ImGui::DragFloat("speed", &tiltSpeed_, 0.01f);
	ImGui::End();
#endif
}

void GamePlayScene::Draw() {
	// 不透明オブジェクトの描画準備
	ctx_.engineContext->object3dCommon->DrawSettingCommon(ctx_.engineContext->textureManager);

	// ステージオブジェクトの描画
	stage_->Draw();

	// 敵の描画処理
	enemyManager_->Draw();

	// 敵の弾の描画処理
	enemyBulletManager_->Draw();

	// 敵の爆弾の描画処理
	enemyBombManager_->Draw();

	// プレイヤーの描画処理
	player_->Draw();

	// パーティクルの描画
	for (auto& particle : enemyDeathEffect_) {
		particle->Draw();
	}

	// 血痕描画
	decalManager_->Draw();

	// フラッシュエフェクト描画
	flashEffect_->Draw();

	// レターボックス描画
	letterBox_->Draw();

	// 半透明オブジェクトの描画準備
	ctx_.engineContext->object3dCommon->DrawSettingTransparent(ctx_.engineContext->textureManager);

	// ガラスの描画
	stage_->DrawTransparent();

	// テキスト描画
	textSprite_->Draw();
}

void GamePlayScene::Finalize() {
	// シーン終了時にエフェクトをデフォルトに戻す
	if (postEffectController_) {
		postEffectController_->Finalize();
	}

	// BGM停止
	audioManager_->StopBGM();
}

void GamePlayScene::GenerateEnemyDeathEffect(const Vector3& pos) {
	// エフェクトの生成
	EffectGenerator::CreateEnemyDeathEffect(ctx_.engineContext, pos, enemyDeathEffect_);
}

void GamePlayScene::UpdateCameraIntro(float deltaTime) {
	switch (introPhase_) {
	case CameraAnimState::WaitingForFade: {
		// フェードマネージャーのタイマーがフェード完了時間を超えたら演出開始
		if (ctx_.fadeManager->GetFadeTimer() >= ctx_.fadeManager->GetFadeDuration()) {
			introPivotAnim_.anim.Start(introStartPivot_, introGoalPivot_, introMoveDuration_, EaseType::EASEINOUTSINE);
			introHoldTimer_.Initialize(introHoldDuration_);
			introPhase_ = CameraAnimState::ToGoal;
		} else {
			// フェード中はカメラの位置を開始地点で固定しておく
			ctx_.currentCamera->SetPivot(introStartPivot_);
			ctx_.currentCamera->UpdateViewMatrix();
		}
		break;
	}
	case CameraAnimState::ToGoal: {
		// ゴール地点へ移動
		bool playing = introPivotAnim_.anim.Update(deltaTime, introPivotAnim_.temp);
		ctx_.currentCamera->SetPivot(introPivotAnim_.temp);
		ctx_.currentCamera->UpdateViewMatrix();

		if (!playing) {
			introHoldTimer_.Initialize(introHoldDuration_);
			introPhase_ = CameraAnimState::Waiting;
		}
		break;
	}
	case CameraAnimState::Waiting: {
		// ゴール地点で少し待機
		introHoldTimer_.Update(deltaTime);
		if (introHoldTimer_.IsEnd()) {
			introPivotAnim_.anim.Start(introGoalPivot_, introStartPivot_, introMoveDuration_, EaseType::EASEINOUTSINE);
			introPhase_ = CameraAnimState::ToStart;
		}
		break;
	}
	case CameraAnimState::ToStart: {
		// 開始地点へ戻る
		bool playing = introPivotAnim_.anim.Update(deltaTime, introPivotAnim_.temp);
		ctx_.currentCamera->SetPivot(introPivotAnim_.temp);
		ctx_.currentCamera->UpdateViewMatrix();

		if (!playing) {
			introPhase_ = CameraAnimState::End;
		}
		break;
	}
	case CameraAnimState::End:
	default:
		// 演出終了
		isIntroPlaying_ = false;
		InitializeGameSceneCamera();
		break;
	}
}

void GamePlayScene::InitializeGameSceneCamera() {
	Vector3 playerPos = player_->GetPosition();
	Vector3 playerRot = player_->GetRotation();
	ctx_.currentCamera->InitializeFollow(playerPos, playerRot, offsetDistance_, cameraPosY_, cameraAngle_);
}