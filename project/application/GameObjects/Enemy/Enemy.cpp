#include "Enemy.h"
#include "EnemyBulletManager.h"

void Enemy::Initialize(EngineContext* ctx) {
	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {3.0f, 0.0f, 0.0f};

	// 描画用インスタンスの生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Hiyoko.obj");

	// AIインスタンス生成&初期化
	ai_ = std::make_unique<EnemyAI>();
	ai_->Initialize(&transform_, ctx);
}

void Enemy::Update(float deltaTime, Vector3 playerPos, EnemyBulletManager* enemyBulletManager) {
	// 当たり判定更新
	Vector3 pos = transform_.translate;
	aabbCol_.max = {pos.x + 0.5f, pos.y, pos.z + 0.5f};
	aabbCol_.min = {pos.x - 0.5f, pos.y, pos.z - 0.5f};

	// AIインスタンス更新
	if (enableMove_) {
		ai_->Update(deltaTime, playerPos, enemyBulletManager);
	}

	if (knockBackAnim_.anim.GetIsActive()) {
		knockBackAnim_.anim.Update(deltaTime, knockBackAnim_.temp);
		transform_.translate = knockBackAnim_.temp;
	}

	// 描画用インスタンス更新処理
	render_->Update(transform_);
}

void Enemy::Draw() {
	// 描画
	render_->Draw();
}

void Enemy::StartKnockBack(Vector3 dir) {
	Vector3 pos = transform_.translate;
	
	// 入力方向ベクトルを正規化
	if (dir.x != 0.0f || dir.z != 0.0f) {
		dir = MathUtility::Normalize(dir);
	} else {
		return;
	}

	Vector3 targetPos = {
	    pos.x + dir.x * knockBackPower_,
	    pos.y,
	    pos.z + dir.z * knockBackPower_,
	};
	knockBackAnim_.anim.Start(transform_.translate, targetPos, 0.5f, EaseType::EASEOUTCUBIC);
}
