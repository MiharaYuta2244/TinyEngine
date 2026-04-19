#include "Wall.h"

void Wall::Initialize(EngineContext* ctx, WallStatus wallStatus) {
	transform_.scale = {wallStatus.width, 1.0f, wallStatus.depth};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {wallStatus.centerX, 0.0f, wallStatus.centerZ};
	wallStatus_ = wallStatus;

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cube.obj");
}

void Wall::Update() {
	// 当たり判定更新
	Vector3 pos = transform_.translate;
	collision_.max = {pos.x + 0.5f, pos.y, pos.z + 0.5f};
	collision_.min = {pos.x - 0.5f, pos.y, pos.z - 0.5f};

	// 描画用インスタンス更新
	render_->Update(transform_);
}

void Wall::Draw() {
	// 描画
	render_->Draw();
}

void Wall::SetWallStatus(WallStatus wallStatus) {
	transform_.scale.x = wallStatus.width;
	transform_.scale.z = wallStatus.depth;
	transform_.translate.x = wallStatus.centerX;
	transform_.translate.z = wallStatus.centerZ;
	wallStatus_ = wallStatus;
}
