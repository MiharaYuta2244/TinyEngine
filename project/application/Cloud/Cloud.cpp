#include "Cloud.h"
#include "Random.h"

void Cloud::Initialize(EngineContext* ctx) {
	cloudModel_ = std::make_unique<Object3d>();
	cloudModel_->Initialize(ctx);
	cloudModel_->SetModel("Cloud2.obj");
	startPos_.x = RandomUtils::RangeFloat(-100.0f, -20.0f);
	startPos_.y = RandomUtils::RangeFloat(20.0f, 25.0f);
	cloudModel_->SetTranslate(startPos_);
	float scale = RandomUtils::RangeFloat(3.0f,6.0f);
	cloudModel_->SetScale({scale, scale, scale});
	cloudModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
	speed_ = RandomUtils::RangeFloat(3.0f, 4.0f);
}

void Cloud::Update(float deltaTime) {
	// 右方向へ移動
	Vector3 pos = cloudModel_->GetTranslate();
	pos.x += speed_ * deltaTime;
	cloudModel_->SetTranslate(pos);

	// X座標の上限
	if (cloudModel_->GetTranslate().x >= 50.0f) {
		cloudModel_->SetTranslate(startPos_); // 初期位置に戻す
	}

	cloudModel_->Update();
}

void Cloud::Draw() { cloudModel_->Draw(); }