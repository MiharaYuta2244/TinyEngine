#include "Enemy.h"
#include "EnemyBulletManager.h"

void Enemy::Initialize(EngineContext* ctx) {
	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {3.0f, 0.0f, 0.0f};

	// 描画用インスタンスの生成&初期化
	render_=std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Hiyoko.obj");

	// AIインスタンス生成&初期化
	ai_ = std::make_unique<EnemyAI>();
	ai_->Initialize(&transform_, ctx);
}

void Enemy::Update(float deltaTime, Vector3 playerPos, EnemyBulletManager* enemyBulletManager) {
	// AIインスタンス更新
	ai_->Update(deltaTime, playerPos, enemyBulletManager);

	// 描画用インスタンス更新処理
	render_->Update(transform_);
}

void Enemy::Draw(){
	// 描画
	render_->Draw();
}