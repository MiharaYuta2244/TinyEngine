#include "PiyoEffect.h"
#include <cmath>
#include <numbers>

void PiyoEffect::Initialize(EngineContext* ctx) {
	for (auto& render : renders_) {
		render = std::make_unique<ObjectRender>();
		render->Initialize(ctx, "plane.obj");
		render->SetTexture("white.png");
		render->SetEnableLighting(false);
		render->SetColor({1.0f, 0.9f, 0.2f, 1.0f});
		render->SetScale({32.0f, 1.0f, 32.0f});
	}
}

void PiyoEffect::Update(float deltaTime, const Vector3& enemyPos) {
	// 回転処理
	angle_ += angularSpeed_ * deltaTime;

	const float step = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kStarCount);
	for (int i = 0; i < kStarCount; ++i) {
		float a = angle_ + step * i;

		Transform t;
		t.scale = {starSize_, starSize_, 1.0f};
		t.rotate = {-std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f}; // Groundと同じく上向き
		t.translate = {enemyPos.x + std::cos(a) * radius_, enemyPos.y + height_, enemyPos.z + std::sin(a) * radius_};
		renders_[i]->Update(t);
	}
}

void PiyoEffect::Draw() {
	// 描画
	for (auto& render : renders_) {
		render->Draw();
	}
}