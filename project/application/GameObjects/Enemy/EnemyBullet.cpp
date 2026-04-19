#include "EnemyBullet.h"

void EnemyBullet::Initialize(EngineContext* ctx, Vector2 dir, Vector3 pos) {
	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = pos;

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cube.obj");

	// 移動方向を受け取る
	direction_ = dir;
}

void EnemyBullet::Update(float deltaTime, float bulletSpeed) {
	// 指定した方向へ移動する
	transform_.translate.x += direction_.x * bulletSpeed * deltaTime;
	transform_.translate.z += direction_.y * bulletSpeed * deltaTime;

	// 描画用インスタンス更新
	render_->Update(transform_);
}

void EnemyBullet::Draw() {
	// 描画
	render_->Draw();
}

bool EnemyBullet::IsDead(const Rect<float>& rect) const { 
	if(rect.CheckOver({transform_.translate.x, transform_.translate.z})){
		return true;
	}

	return false; 
}
