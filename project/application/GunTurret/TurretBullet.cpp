#include "TurretBullet.h"
#include "MathOperator.h"

TurretBullet::TurretBullet(EngineContext* ctx, Vector3 pos) {
	Initialize(ctx, pos);
}

void TurretBullet::Initialize(EngineContext* ctx, Vector3 pos) {
	bulletModel_ = std::make_unique<Object3d>();
	bulletModel_->Initialize(ctx);
	bulletModel_->SetModel("sphere.obj");
	bulletModel_->SetTranslate(pos);
	bulletModel_->SetScale({0.2f, 0.2f, 0.2f});
	bulletModel_->SetRotate({0.0f, 0.0f, 0.0f});
}

void TurretBullet::Update(float deltaTime, Vector3 direction) {
	// 移動処理
	Move(deltaTime, direction);

	// Object3dの更新
	bulletModel_->Update();
}

void TurretBullet::Draw() { bulletModel_->Draw(); }

void TurretBullet::Move(float deltaTime, Vector3 direction) { bulletModel_->SetTranslate(bulletModel_->GetTranslate() += direction * deltaTime); }
