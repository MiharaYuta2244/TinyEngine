#include "Player.h"
#include "ColorPalette.h"
#include "GameObjects/Effect/EffectGenerator.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "GameObjects/PlayTestConfig.h"
#include "MathOperator.h"
#include "MathUtility.h"

using namespace TinyEngine;

void Player::Initialize(EngineContext* ctx, TinyEngine::DecalManager* bloodDecalManager) {
	ctx_ = ctx;

	transform_.scale = {3.0f, 3.0f, 3.0f};
	transform_.rotate = {0.0f, std::numbers::pi_v<float> / 2.0f, 0.0f};
	transform_.translate = {0.0f, 0.0f, 0.0f};

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "GorillaIdle.gltf");
	KeyframeAnimation keyframeAnimation;
	Animation sneakWalkAnimation = keyframeAnimation.LoadAnimationFile("GorillaIdle.gltf");
	render_->GetObject3d()->PlayAnimation(sneakWalkAnimation);
	render_->SetColor(color_);
	render_->SetTransform(transform_);
	render_->SetIsSkinning(true);

	// 移動用インスタンス生成
	move_ = std::make_unique<PlayerMove>();

	// HP管理用インスタンス生成&初期化
	hp_ = std::make_unique<PlayerHealth>();
	hp_->Initialize(maxHP_);

	// 入力用インスタンス生成
	input_ = std::make_unique<PlayerInput>();

	// アニメーション用インスタンス生成
	animator_ = std::make_unique<PlayerAnimator>();
	animator_->Initialize(render_.get());

	// 血痕の管理インスタンスポインタ
	bloodDecalManager_ = bloodDecalManager;

	// 投げ軌道プレビューの生成&初期化
	throwPreview_ = std::make_unique<ThrowPreview>();
	throwPreview_->Initialize(bloodDecalManager_);

	// プレイヤーアクション用インスタンスの生成
	grab_ = std::make_unique<PlayerGrab>();

	// オーディオマネージャー
	audioManager_ = std::make_unique<AudioManager>();
	audioManager_->Initialize();
	audioManager_->LoadWave("Heal", "resources/sounds/se/Heal.mp3");

	// 振動マネージャーの生成
	rumbleManager_ = std::make_unique<GameRumbleController>();
}

void Player::Update(
    float deltaTime, DirectInput* input, GamePad* gamePad, EnemyManager* enemyManager, const std::list<std::unique_ptr<Soap>>& soaps,
    const std::list<std::unique_ptr<Barricade>>& barricades) {
	// RumbleManagerの初期化と更新
	rumbleManager_->Initialize(gamePad);
	rumbleManager_->Update(deltaTime);

	// 音声更新
	audioManager_->Update();

	// 入力の取得
	grab_->ValidateBarricade(barricades);
	const bool wasHoldingBarricade = grab_->IsHoldingBarricade();
	const PlayerInputState in = input_->Poll(input, gamePad, ctx_, transform_.translate, grab_->IsHolding());
	isAttackTriggered_ = in.attackPressed;

	// 移動状態の更新と向きの記録
	const float length = MathUtility::Length(in.moveDir);
	if (length > 0.05f) {
		// 正規化して記録（攻撃・投げの方向用）
		lastMoveDirection_ = {in.moveDir.x / length, in.moveDir.y / length};
		isMoving_ = true;
	} else {
		isMoving_ = false;
	}

	// 基本速度
	speedMultiplier_ = 1.0f;

	// 掴んでいる場合は速度を低下
	if (grab_->IsHolding()) {
		speedMultiplier_ *= 0.3f;
	}

	if (!hp_->IsDead()) {
		// 移動入力がある場合のみ方向による速度補正
		if (length > 0.05f) {

			// プレイヤーが向いている方向
			Vector2 forward = {std::sinf(transform_.rotate.y), std::cosf(transform_.rotate.y)};

			// 移動方向を正規化
			Vector2 moveDir = {in.moveDir.x / length, in.moveDir.y / length};

			// 向いている方向と移動方向の内積
			float dot = forward.x * moveDir.x + forward.y * moveDir.y;

			// 内積から速度倍率を計算
			float directionSpeedMultiplier = -0.05f * dot * dot + 0.25f * dot + 0.8f;

			// 掴み中の速度倍率に加えて適用
			speedMultiplier_ *= directionSpeedMultiplier;
		}

		// 移動更新
		const Vector2 moveDir = grab_->IsPullingBarricade() && in.grabHeld ? Vector2{} : in.moveDir;
		move_->Update(&transform_, moveDir, in.aimDir, deltaTime, speedMultiplier_, grab_->IsHolding());

		// 攻撃等に使うベクトルの更新
		lastMoveDirection_.x = std::sinf(transform_.rotate.y);
		lastMoveDirection_.y = std::cosf(transform_.rotate.y);
	}

	// HP管理インスタンス更新
	hp_->Update(deltaTime);

	if (ctx_ && ctx_->object3dCommon) {
		PointLight pointLight = ctx_->object3dCommon->GetPointLight();

		// パラメータ
		pointLight.position = transform_.translate;
		pointLight.position.y += pointLightHeightOffset_;
		pointLight.color = pointLightColor_;
		pointLight.intensity = pointLightIntensity_;
		pointLight.radius = pointLightRadius_;
		pointLight.decay = pointLightDecay_;

		ctx_->object3dCommon->SetPointLightParam(pointLight);
	}

	// 衝突判定のために、移動後の座標で仮のAABBの更新
	UpdateCollision();

	// 掴み、攻撃の入力フラグ
	PlayerGrab::Input grabInput;
	grabInput.grabHeld = in.grabHeld;
	grabInput.grabPressed = in.grabPressed;
	grabInput.grabReleased = in.grabReleased;
	grabInput.attackPressed = in.attackPressed;
	grabInput.moveDir = in.moveDir;

	PlayerGrab::Context grabCtx;
	grabCtx.position = transform_.translate;
	grabCtx.rotate = transform_.rotate;
	grabCtx.forward = lastMoveDirection_;
	grabCtx.enableAttack = enableAttack_;
	grabCtx.deltaTime = deltaTime;

	PlayerGrab::Result grabResult;
	if (!hp_->IsDead()) {
		grabResult = grab_->Update(grabInput, grabCtx, enemyManager, soaps, barricades);
	}
	if (wasHoldingBarricade || grabResult.grabbedBarricade) {
		isAttackTriggered_ = false;
	}

	// GamePlaySceneがカメラシェイクに使う
	if (grabResult.grabbedEnemy || grabResult.grabbedSoap || grabResult.grabbedBarricade) {
		isGrabTriggered_ = true;
	}

	if (grabResult.grabbedEnemy || grabResult.grabbedBarricade) {
		animator_->PlayAction(PlayerAnim::Hold);
	}

	if (grabResult.attacked) {
		animator_->PlayAction(PlayerAnim::Push);
	}

	// 掴んだor投げた瞬間はプレイヤーの速度を0にする
	if (grabResult.stopMovement) {
		move_->ResetVelocity();
	}

	// アニメーションの更新
	animator_->Update(deltaTime, isMoving_, grab_->IsHolding());

	// ヒットエフェクトの更新
	for (auto& particle : hitEffects_) {
		particle->Update();
	}

	// ヒットエフェクト削除
	std::erase_if(hitEffects_, [](const std::unique_ptr<TinyEngine::Particle>& p) { return p->IsFinished(); });

	// 回復エフェクトの更新
	for (auto& particle : healEffects_) {
		particle->UpdateTranslate(transform_.translate);
		particle->Update();
	}

	// 回復エフェクト削除
	std::erase_if(healEffects_, [](const std::unique_ptr<TinyEngine::Particle>& p) { return p->IsFinished(); });

	// 出血処理
	Bleeding(deltaTime);

	// 投げ軌道プレビューの更新
	UpdateThrowPreview(deltaTime);

	// ギズモ用当たり判定更新
	UpdateAABBForGizmo();
}

void Player::PostUpdate() {
	// 押し戻しによってtransform_が変更された場合、それに合わせてAABBも最終的な正しい位置に合わせる
	UpdateCollision();

	grab_->PlaceShield(transform_.translate, lastMoveDirection_, transform_.rotate.y);
	if (hp_->IsDead()) {
		grab_->ReleaseBarricade();
	}

	// 最後に描画用の更新を行う
	render_->Update(transform_);
}

void Player::Draw() {
	// ヒットエフェクトパーティクルの描画
	for (auto& particle : hitEffects_) {
		particle->Draw();
	}

	// 回復エフェクトの描画
	for (auto& particle : healEffects_) {
		particle->Draw();
	}

	// 描画
	render_->Draw();
}

bool Player::IsDead() const { return hp_->IsDead(); }

bool Player::BlocksWithShield(const Vector3& towardAttack) const {
	if (!grab_->IsHoldingShield() || hp_->IsDead()) {
		return false;
	}
	// 前方の左右60度をガード範囲にする
	const float dot = lastMoveDirection_.x * towardAttack.x + lastMoveDirection_.y * towardAttack.z;
	const float lengthSq = towardAttack.x * towardAttack.x + towardAttack.z * towardAttack.z;
	return dot > 0.0f && dot * dot >= 0.25f * lengthSq;
}

void Player::Damage(float value) {
	if (hp_->GetIsInvincible())
		return;

	// ダメージ前のHPを保存
	float beforeHP = hp_->GetCurrentHP();

	// ダメージ処理
	hp_->Damage(value);

	// ダメージ後のHP
	float afterHP = hp_->GetCurrentHP();

	// ヒットエフェクト生成
	GenerateHitEffect();

	// 血痕の生成
	AddBloodDecal({4, 4, 1});

	// 出血用のタイマー初期化
	bleedingTimer_.Initialize(0.5f);

	// 被弾振動呼び出し
	rumbleManager_->TriggerPlayerDamage();
}

void Player::Heal(float value) {
	// 回復前のHPを保存
	float beforeHP = hp_->GetCurrentHP();

	// 回復処理
	hp_->Heal(value);

	// 回復後のHP
	float afterHP = hp_->GetCurrentHP();

	if (beforeHP != afterHP) {
		// エフェクト生成
		GenerateHealEffect();

		// 音声再生
		audioManager_->PlaySE("Heal", 1.0f);
	}
}

void Player::AllHeal() {
	// 回復前のHPを保存
	float beforeHP = hp_->GetCurrentHP();

	// 全回復処理
	hp_->AllHeal();

	// 回復後のHP
	float afterHP = hp_->GetCurrentHP();

	if (beforeHP != afterHP) {
		// エフェクト生成
		GenerateHealEffect();

		// 音声再生
		audioManager_->PlaySE("Heal", 0.3f);
	}
}

void Player::UpdateCollision() {
	// 攻撃用の当たり判定更新
	Vector3 pos = transform_.translate;
	Vector3 forward = {std::sin(transform_.rotate.y), 0.0f, std::cos(transform_.rotate.y)};
	Vector3 right = {forward.z, 0.0f, -forward.x};
	Vector3 up = {0.0f, 1.0f, 0.0f};

	float attackLength = 1.4f; // 前方に伸びる距離
	float attackWidth = 0.4f;  // 横幅
	float attackHeight = 1.0f; // 高さ

	Vector3 center = {pos.x + forward.x * attackLength * 0.5f, pos.y, pos.z + forward.z * attackLength * 0.5f};
	attackCol_.center = center;
	attackCol_.orientations[0] = right;
	attackCol_.orientations[1] = up;
	attackCol_.orientations[2] = forward;
	attackCol_.size = {attackWidth, attackHeight, attackLength};

	// 本体の当たり判定更新
	bodyCol_.max = {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f};
	bodyCol_.min = {pos.x - 0.5f, pos.y - 0.5f, pos.z - 0.5f};
}

void Player::GenerateHitEffect() {
	if (preHP_ == hp_->GetCurrentHP())
		return;

	// エフェクトの生成
	EffectGenerator::CreateHitEffect(ctx_, transform_.translate, hitEffects_);

	preHP_ = hp_->GetCurrentHP();
}

void Player::AddBloodDecal(Vector3 scale) {
	Vector3 basePos = transform_.translate;
	Vector3 finalPos;
	finalPos.x = basePos.x;
	finalPos.y = 0.1f;
	finalPos.z = basePos.z;
	bloodDecalManager_->AddDecal("Blood.png", finalPos, {std::numbers::pi_v<float> / 2.0f, 0, 0}, scale, ColorPalette::Orange(), 4.0f, 2.0f);
}

void Player::Bleeding(float deltaTime) {
	if (hp_->GetCurrentHP() <= 2 && isMoving_) {
		if (bleedingTimer_.IsEnd()) {
			Vector3 scale = {0.0f, 0.0f, 0.0f};
			float interval = 0.0f;

			if (PlaytestConfig::obscureMode) {
				// HPに依存しない固定値
				scale = {0.75f, 0.75f, 0.75f};
				interval = 0.35f;
			} else {
				// スケールの最大値と最小値
				Vector3 bloodScaleMin = {0.5f, 0.5f, 0.5f};
				Vector3 bloodScaleMax = {1.0f, 1.0f, 1.0f};

				// 生成間隔の最大値と最小値
				float intervalMin = 0.2f;
				float intervalmax = 0.5f;

				float t = hp_->GetCurrentHP() / 2.0f;

				// スケールの確定
				scale = bloodScaleMax - (bloodScaleMax - bloodScaleMin) * t;

				// 生成間隔の確定
				interval = intervalMin + (intervalmax - intervalMin) * t;
			}

			// 血痕の生成
			AddBloodDecal(scale);

			// 出血用のタイマー初期化
			bleedingTimer_.Initialize(interval);
		}

		// 出血用タイマー更新
		bleedingTimer_.Update(deltaTime);
	}
}

void Player::GenerateHealEffect() {
	// エフェクトの生成
	EffectGenerator::CreateHealEffect(ctx_, transform_.translate, healEffects_);

	// 追従フラグを立てる
	for (auto& effect : healEffects_) {
		effect->SetFollowTarget(true);
	}
}

void Player::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Player PointLight");

	ImGui::DragFloat("Height Offset", &pointLightHeightOffset_, 0.1f, -5.0f, 10.0f);
	ImGui::ColorEdit4("Color", &pointLightColor_.x);
	ImGui::DragFloat("Intensity", &pointLightIntensity_, 0.05f, 0.0f, 20.0f);
	ImGui::DragFloat("Radius", &pointLightRadius_, 0.5f, 0.0f, 100.0f);
	ImGui::DragFloat("Decay", &pointLightDecay_, 0.05f, 0.0f, 10.0f);

	ImGui::End();

	// 振動用のImGui描画呼び出し
	if (rumbleManager_) {
		rumbleManager_->DrawImGui();
	}
#endif
}

void Player::UpdateThrowPreview(float deltaTime) {
	Vector3 pos{};
	float power = 0.0f;
	float friction = 1.0f;

	// 何も掴んでいなければプレビューを表示しない
	if (!grab_->GetPreviewParams(enableAttack_, pos, power, friction)) {
		throwPreview_->Hide();
		return;
	}

	// 軌道プレビューの更新
	throwPreview_->Update(deltaTime, pos, lastMoveDirection_, power, friction);
}
