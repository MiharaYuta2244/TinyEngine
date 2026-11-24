#include "Enemy.h"
#include "EffectLibrary.h"
#include "MathOperator.h"
#include "ChargeModule.h"
#include "ColorPalette.h"
#include "Effect/EffectGenerator.h"
#include "GameObjects/PlayTestConfig.h"
#include "GameObjects/Player/Player.h"
#include "Gameobjects/Enemy/Behavior/EnemyBehaviorFactory.h"
#include "Gameobjects/Enemy/Weapon/EnemyBulletManager.h"

using namespace TinyEngine;

namespace {
const char* GetEnemyModel(EnemyType type) {
	switch (type) {
	case EnemyType::Shotgun:
		return "EnemyShotgun.gltf";
	case EnemyType::Bomber:
		return "EnemyBomber.gltf";
	case EnemyType::Assault:
		return "EnemyAssault.gltf";
	default:
		return "EnemyNormal.gltf";
	}
}

const char* GetGunModel(EnemyType type) { return type == EnemyType::Shotgun ? "EnemyShotgunGun.obj" : "EnemyPistol.obj"; }
} // namespace

int Enemy::index = 0;

Enemy::Enemy() { id_ = index++; }

void Enemy::Initialize(EngineContext* ctx, Vector3 pos, EnemyType type, DecalManager* bloodDecalManager, AudioManager* audioManager) {
	ctx_ = ctx;
	audioManager_ = audioManager;
	ApplyType(type);

	transform_.scale = {0.4f, 0.4f, 0.4f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = pos;

	// 描画用インスタンスの生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, GetEnemyModel(GetEnemyType()));
	render_->SetTransform(transform_);
	render_->SetEnvScale(envScale_);
	render_->SetIsSkinning(true);

	// アニメーターの初期化
	animator_.Initialize(render_->GetObject3d());

	// 銃モデルの生成&初期化
	renderGun_ = std::make_unique<ObjectRender>();
	renderGun_->Initialize(ctx, GetGunModel(GetEnemyType()));
	renderGun_->SetTransform(transform_);
	renderGun_->SetEnvScale(envScale_);

	// AIインスタンス生成&初期化
	ai_ = std::make_unique<EnemyAI>();
	ai_->Initialize(&transform_, ctx, behavior_.get(), audioManager_);

	// 視界インスタンス生成&初期化
	visionCone_ = std::make_unique<VisionCone>();
	Visionparam param = ai_->GetVisionParam();
	visionCone_->Initialize(ctx, param.radius, param.angle);

	// 血痕
	bloodDecalManager_ = bloodDecalManager;
}

void Enemy::Update(
    float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, const std::vector<AABB>& obstacles, DoorManager* doorManager, GlassManager* glassManager, EnemyBombManager* enemyBombManager,
    const std::optional<Vector3>& distraction) {
	if (isDead_)
		return;

	// ノックバックの更新処理
	UpdateKnockback(deltaTime);

	// ダウン時(ピヨピヨ状態)の処理
	if (isDown_) {
		downTimer_.Update(deltaTime);

		// ピヨピヨの更新
		if (piyoEffect_) {
			piyoEffect_->Update(deltaTime, transform_.translate);
		}

		if (downTimer_.IsEnd()) {
			isDown_ = false;
			piyoEffect_.reset(); // 復帰したら削除

			// 復帰時に掴まれている場合はHold状態を壊さない
			if (ai_->GetState() == EnemyAI::State::Down) {
				ai_->SetState(EnemyAI::State::Normal);
				ai_->ResetShotTimer();
			}
		}

		UpdateCollision();
		UpdateAABBForGizmo();
		return;
	}

	for (auto& particle : burnParticles_) {
		particle->SetTranslate(transform_.translate + Vector3{0.0f, 0.8f, 0.0f});
		particle->Update();
	}
	std::erase_if(burnParticles_, [](const auto& particle) { return particle->IsFinished(); });

	// タイマーの更新(無敵・点滅)
	UpdateTimers(deltaTime);

	// エフェクトの更新
	UpdateEffects(deltaTime);

	// 停止状態の時は早期リターン
	if (!isMove_ && !IsBurning()) {
		return;
	}

	// AI更新前の座標を記録
	Vector3 prevPos = transform_.translate;

	// AIインスタンス更新
	ai_->Update(deltaTime, player, enemyBulletManager, obstacles, doorManager, glassManager, enemyBombManager, distraction);

	// アニメーション更新処理
	UpdateAnimation(prevPos);

	// プレイヤー発見時に「!」マークの生成
	if (lastState != ai_->GetState() && ai_->GetState() == EnemyAI::State::Vigilance) {
		GenerateExMark();
	}

	// 状態に応じた視界の更新処理
	UpdateVisionCone(obstacles, doorManager, glassManager);

	// 「!」マークのアニメーションが終了していれば「!」マークインスタンスを削除
	if (exclamationMark_) {
		if (!exclamationMark_->IsFinishedAnimation()) {
			exclamationMark_.reset();
			exclamationMark_ = nullptr;
		}
	}

	// 「!」マークの更新
	if (exclamationMark_) {
		exclamationMark_->Update(deltaTime, transform_.translate);
	}

	// 敵AIの状態を記録
	lastState = ai_->GetState();

	// 当たり判定更新　衝突判定用
	UpdateCollision();

	// ギズモ用当たり判定更新
	UpdateAABBForGizmo();
}

void Enemy::PostUpdate() {
	// エディターでの変更は、次のスキニング処理を記録する前に反映する
	if (appearanceDirty_) {
		const EnemyType type = GetEnemyType();
		render_->SetModel(GetEnemyModel(type));
		const char* gunModel = GetGunModel(type);
		if (renderGun_->GetFilepath() != gunModel) {
			renderGun_->SetModel(gunModel);
		}
		appearanceDirty_ = false;
	}

	// 押し戻しを反映したAABBの再計算
	UpdateCollision();

	// 正しい座標で描画用インスタンスの更新
	render_->Update(transform_);

	// ボーン座標が最新になった後で足跡を置く
	UpdateFootprint();

	// 銃はグリップを原点にし、銃口をローカルの+Zに向けている
	const Vector3 hand = render_->GetObject3d()->GetBonePos(L"ボーン.007", true);
	const Transform gunPose{transform_.scale, {0.0f, transform_.rotate.y, 0.0f}, hand};
	renderGun_->Update(gunPose);

	if (!IsDead()) {
		// OBJの銃口面の中心。ショットガンは左右の銃口の間を射線の起点にする
		Vector3 muzzleOffset = GetEnemyType() == EnemyType::Shotgun ? Vector3{0.0f, 0.49f, 3.38f} : Vector3{0.0f, 0.45f, 1.57f};
		auto gunWorld = MathUtility::MakeAffineMatrix(gunPose.scale, gunPose.rotate, gunPose.translate);
		const Vector3 muzzle = MathUtility::Transform(muzzleOffset, gunWorld);

		if (const auto direction = ai_->FirePendingShot(muzzle, GetCollisionId())) {
			EffectGenerator::CreateMuzzleFlash(ctx_, muzzle, *direction, muzzleParticles_);
		}
	}
}

void Enemy::Draw() {
	if (!isDead_) {
		// 描画
		if (isBlinkVisible_) {
			render_->Draw();
			renderGun_->Draw();
		}

		// 「!」マークの描画
		if (!isDown_ && isMove_ && exclamationMark_) {
			exclamationMark_->Draw();
		}

		// ピヨピヨの描画
		if (isDown_ && piyoEffect_) {
			piyoEffect_->Draw();
		}

		for (auto& particle : burnParticles_) {
			particle->Draw();
		}

		// マズルフラッシュパーティクルの描画
		for (auto& particle : muzzleParticles_) {
			particle->Draw();
		}

		// チャージパーティクルの描画
		if (!isDown_ && isMove_ && chargeParticle_) {
			chargeParticle_->Draw();
		}

		// シリンダーチャージパーティクル描画
		if (!isDown_ && isMove_ && chargeCylinderEffect_) {
			chargeCylinderEffect_->Draw();
		}
	}
}

void Enemy::DrawVision() {
	if (isDead_)
		return;

	// 視界の描画
	if (enableMove_ && !isDown_ && !IsBurning()) {
		visionCone_->Draw();
	}
}

void Enemy::StartKnockBack(Vector3 dir) {
	Vector3 pos = transform_.translate;

	// 入力方向ベクトルを正規化
	if (dir.x != 0.0f || dir.z != 0.0f) {
		dir = MathUtility::Normalize(dir);
	} else {
		return;
	}

	// 瞬時に初速を与える
	velocity_ = {dir.x * knockBackPower_, 0.0f, dir.z * knockBackPower_};
}

void Enemy::Kill() {
	if (isDead_)
		return;

	isDead_ = true;

	// 血痕の生成
	AddBloodDecal();
}

void Enemy::Damage() {
	if (isDead_)
		return;

	// 無敵時間中は早期リターン
	if (invincibleTimer_ > 0.0f) {
		return;
	}

	hp_--;
	if (hp_ <= 0) {
		Kill();
	} else {
		ai_->SetState(EnemyAI::State::Normal);
		ai_->ResetShotTimer();

		// 血痕の生成
		AddBloodDecal();
	}

	damageBlinkTimer_ = 1.0f; // 点滅させる時間を設定
	isBlinkVisible_ = true;   // 点滅用のフラグを立てる
	invincibleTimer_ = 0.5f;  // 無敵時間の設定
}

void Enemy::UpdateCollision() {
	Vector3 pos = transform_.translate;
	bodyCol_.max = {pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f};
	bodyCol_.min = {pos.x - 0.5f, pos.y - 0.5f, pos.z - 0.5f};
}

void Enemy::GenerateExMark() {
	// 「!」マークインスタンス生成&初期化
	exclamationMark_ = std::make_unique<ExclamationMark>();
	Vector3 pos = {transform_.translate.x, transform_.translate.y + 1.0f, transform_.translate.z};
	exclamationMark_->Initialize(ctx_, transform_.translate);
}

void Enemy::SetAIState(EnemyAI::State state) {
	if (ai_) {
		ai_->SetState(state);
	}
}

void Enemy::StopKnockback() { velocity_ = {0.0f, 0.0f, 0.0f}; }

void Enemy::AddBloodDecal() {
	Vector3 basePos = transform_.translate;
	Vector3 finalPos;
	finalPos.x = basePos.x;
	finalPos.y = 0.1f;
	finalPos.z = basePos.z;
	bloodDecalManager_->AddDecal("Blood.png", finalPos, {std::numbers::pi_v<float> / 2.0f, 0, 0}, {4, 4, 1}, ColorPalette::DarkRed(), 4.0f, 2.0f);
}

void Enemy::ApplyType(EnemyType type) {
	if (render_ && GetEnemyType() != type) {
		appearanceDirty_ = true;
	}

	behavior_ = EnemyBehaviorFactory::Create(type);
	hp_ = behavior_->GetMaxHP();

	if (ai_) {
		ai_->SetBehavior(behavior_.get());
	}
}

void Enemy::Ignite() {
	if (isDead_ || IsBurning()) {
		return;
	}

	ai_->Ignite();
	isDown_ = false;
	piyoEffect_.reset();
	exclamationMark_.reset();
	chargeParticle_.reset();
	chargeCylinderEffect_.reset();
	StopKnockback();
	burnParticles_.clear();
	EffectLibrary::Get().Spawn(ctx_, "EnemyBurning", transform_.translate + Vector3{0.0f, 0.8f, 0.0f}, burnParticles_);
	for (auto& particle : burnParticles_) {
		particle->SetEmitMode(false, EnemyAI::kBurnDuration);
	}
}

void Enemy::StartDown(float duration) {
	if (isDead_ || isDown_ || IsBurning()) {
		return;
	}

	isDown_ = true;
	downDuration_ = duration;
	downTimer_.Initialize(downDuration_);

	ai_->SetState(EnemyAI::State::Down);
	ai_->ResetShotTimer();

	// チャージエフェクトや「!」マークを消去
	exclamationMark_.reset();
	chargeParticle_.reset();
	chargeCylinderEffect_.reset();

	// 吹き飛び中などの速度も止めておく
	velocity_ = {0.0f, 0.0f, 0.0f};

	// ダウン開始時にピヨピヨSEを再生
	if (audioManager_) {
		audioManager_->PlaySE("Piyo", 0.3f);
	}

	// ピヨピヨ演出の開始
	piyoEffect_ = std::make_unique<PiyoEffect>();
	piyoEffect_->Initialize(ctx_);
}

void Enemy::PlayDeathEffect(std::list<std::unique_ptr<Particle>>& container) { behavior_->OnDeath(ctx_, transform_.translate, container); }

void Enemy::UpdateFootprint() {
	if (!footprintManager_) {
		return;
	}

	// 初回は基準位置を記録するだけ
	if (!hasLastFootprintPos_) {
		lastFootprintCheckPos_ = transform_.translate;
		hasLastFootprintPos_ = true;
		return;
	}

	Vector3 delta = transform_.translate - lastFootprintCheckPos_;
	delta.y = 0.0f;
	lastFootprintCheckPos_ = transform_.translate;

	// ノックバック・ダウン・拘束中は歩いていないので足跡を残さない
	if (isDown_ || IsKnockBack() || ai_->GetState() == EnemyAI::State::Hold) {
		strideAccum_ = 0.0f;
		return;
	}

	strideAccum_ += MathUtility::Length(delta);
	if (strideAccum_ < strideLength_) {
		return;
	}
	strideAccum_ -= strideLength_;

	// 左右交互にその足のボーン座標へ配置
	const std::wstring boneName = isNextFootLeft_ ? L"ボーン.011" : L"ボーン.019";
	Vector3 footPos = render_->GetBonePos(boneName);

	// ボーンが見つからない場合はGetBonePosが(0,0,0)を返すので、本体座標で代用
	if (footPos.x == 0.0f && footPos.y == 0.0f && footPos.z == 0.0f) {
		footPos = transform_.translate;
	}

	footprintManager_->Add(footPos, transform_.rotate.y, isNextFootLeft_);
	isNextFootLeft_ = !isNextFootLeft_;
}

void Enemy::UpdateTimers(float deltaTime) {
	// 無敵タイマー更新
	if (invincibleTimer_ > 0.0f) {
		invincibleTimer_ -= deltaTime;
	}

	// 点滅タイマー更新
	if (damageBlinkTimer_ > 0.0f) {
		damageBlinkTimer_ -= deltaTime;

		constexpr float blinkInterval = 0.05f;

		if (fmod(damageBlinkTimer_, blinkInterval * 2) < blinkInterval) {
			isBlinkVisible_ = false;
		} else {
			isBlinkVisible_ = true;
		}
	} else {
		isBlinkVisible_ = true;
	}
}

void Enemy::UpdateEffects(float deltaTime) {
	// マズルフラッシュパーティクルの更新と自動削除
	for (auto& particle : muzzleParticles_) {
		particle->Update();
	}
	std::erase_if(muzzleParticles_, [](const std::unique_ptr<Particle>& p) { return p->IsFinished(); });

	// チャージエフェクトはAIの「チャージ中」判定に完全に同期させる
	const bool isCharging = ai_->IsCharging();

	// 生成
	if (isCharging && !chargeParticle_) {
		chargeParticle_ = std::make_unique<Particle>();
		chargeParticle_->Initialize(ctx_, transform_.translate, "SolarGlow.png", std::make_unique<ChargeModule>(), nullptr, ParticleMeshType::Square);
		chargeParticle_->SetEmitMode(true, 0.1f);
		chargeParticle_->SetEmitterParam(3, 0.05f);

		chargeCylinderEffect_ = std::make_unique<Particle>();
		chargeCylinderEffect_->Initialize(ctx_, transform_.translate, "gradationLine.png", std::make_unique<ChargeCylinderModule>(ai_.get(), &transform_), nullptr, ParticleMeshType::Cylinder);
		chargeCylinderEffect_->SetEmitMode(false, 0.0f);
		chargeCylinderEffect_->SetEmitterParam(1, 9999.0f);
	}

	// 削除
	if (!isCharging && chargeParticle_) {
		chargeParticle_.reset();
		chargeCylinderEffect_.reset();
	}

	// 更新
	if (chargeParticle_) {
		chargeParticle_->SetTranslate(transform_.translate);
		chargeParticle_->Update();
	}
	if (chargeCylinderEffect_) {
		chargeCylinderEffect_->Update();
	}
}

void Enemy::UpdateVisionCone(const std::vector<AABB>& obstacles, DoorManager* doorManager, GlassManager* glassManager) {
	const bool obscure = PlaytestConfig::obscureMode;
	visionCone_->SetIgnoreObstacles(obscure);

	// AIの状態を取得して色を変える
	if (ai_->GetState() == EnemyAI::State::Vigilance) {
		// 警戒状態なら赤色
		visionCone_->SetColor({1.0f, 0.0f, 0.0f, 0.3f});
		// 射撃ゲージの進行度を渡す
		visionCone_->SetChargeProgress(obscure ? 0.0f : ai_->GetShotProgress());
	} else if (ai_->GetState() == EnemyAI::State::Hold) {
		// 拘束状態も射撃するのでプログレスを渡す
		visionCone_->SetColor({1.0f, 0.5f, 0.0f, 0.3f});
		visionCone_->SetChargeProgress(obscure ? 0.0f : ai_->GetShotProgress());
	} else {
		// 通常状態なら緑色
		visionCone_->SetColor({0.0f, 1.0f, 0.0f, 0.3f});
		// プログレスはリセット
		visionCone_->SetChargeProgress(0.0f);
	}

	// 視界
	visionCone_->SetTranslate(transform_.translate);
	visionCone_->SetRotate(transform_.rotate);
	visionCone_->Update(obstacles, doorManager->GetObjects(), glassManager->GetObjects());
}

void Enemy::UpdateKnockback(float deltaTime) {
	// 速度による移動と減衰
	if (velocity_.x != 0.0f || velocity_.z != 0.0f) {
		// 速度による座標の更新
		transform_.translate.x += velocity_.x * deltaTime;
		transform_.translate.z += velocity_.z * deltaTime;

		// 摩擦処理
		velocity_.x = MathUtility::Lerp(velocity_.x, 0.0f, knockBackFriction_ * deltaTime);
		velocity_.z = MathUtility::Lerp(velocity_.z, 0.0f, knockBackFriction_ * deltaTime);

		// 速度が十分に小さくなったら完全に停止させる
		if (std::abs(velocity_.x) < 0.1f && std::abs(velocity_.z) < 0.1f) {
			velocity_ = {0.0f, 0.0f, 0.0f};
		}
	}
}

void Enemy::UpdateAnimation(Vector3 prevPos) {
	const float dx = transform_.translate.x - prevPos.x;
	const float dz = transform_.translate.z - prevPos.z;
	animator_.Update(ai_->GetState(), (dx * dx + dz * dz) > 0.0001f); // 移動時のみ更新
}

void Enemy::OnCollisionEnter(const CollisionEvent& event, const CollisionContext& context) {
	if (IsDead()) {
		return;
	}

	if (event.other.CompareTag(CollisionTag::Barricade) && event.otherPart == CollisionPart::Thrown) {
		// バリケードに投げられると死亡
		Kill();
	} else if (event.other.CompareTag(CollisionTag::EnemyBullet)) {
		// 敵の弾に当たったらダメージ
		Damage();
	}

	// ダメージ判定後死亡したら
	if (IsDead()) {
		context.onEnemyDeath(*this);
	}
}

void Enemy::OnCollisionStay(const CollisionEvent& event, const CollisionContext& context) {
	if (IsDead()) {
		return;
	}

	switch (event.other.GetTag()) {
	case CollisionTag::Wall:
	case CollisionTag::Door:
	case CollisionTag::Barricade:
		// ノックバック中にステージオブジェクトに当たると死亡
		if (event.otherPart == CollisionPart::Body && IsKnockBack()) {
			Kill();
			context.onEnemyDeath(*this);
		}
		break;
	case CollisionTag::Enemy: {
		auto& other = static_cast<Enemy&>(event.other);
		if (other.IsDead() || (!IsKnockBack() && !other.IsKnockBack())) {
			break;
		}

		// ノックバック状態の敵と当たるとダメージ処理
		Damage();
		other.Damage();
		StopKnockback();
		other.StopKnockback();
		
		// 死んだ場合
		if (IsDead()) {
			context.onEnemyDeath(*this);
		}

		// 当たった相手が死んだ場合
		if (other.IsDead()) {
			context.onEnemyDeath(other);
		}

		break;
	}
	default:
		break;
	}
}

void Enemy::ApplyCollisionDisplacement(const Vector3& displacement) {
	transform_.translate += displacement;
	UpdateCollision();
}
