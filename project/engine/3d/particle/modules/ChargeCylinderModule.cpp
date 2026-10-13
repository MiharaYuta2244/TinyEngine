#include "ChargeCylinderModule.h"
#include <algorithm>
#include <cmath>

ChargeCylinderModule::ChargeCylinderModule(EnemyAI* ai, Transform* enemyTransform, float maxRadius, float minRadius, float height)
    : ai_(ai), enemyTransform_(enemyTransform), maxRadius_(maxRadius), minRadius_(minRadius), height_(height), pulsePhase_(0.0f) {}

void ChargeCylinderModule::Initialize(ParticleState& particle, EngineContext* ctx) {
	particle.lifeTime = 9999.0f;
	particle.currentTime = 0.0f;

	// 初期スケール
	particle.transform.scale = {maxRadius_, height_, maxRadius_};

	// パーティクルの色
	particle.color = {1.0f, 0.2f, 0.2f, 1.0f};
}

void ChargeCylinderModule::Update(ParticleState& particle, float deltaTime, EngineContext* ctx) {
	if (!ai_ || !enemyTransform_)
		return;

	// 敵の座標に追従させる
	particle.transform.translate = enemyTransform_->translate;

	// 発射タイマーの割合
	float timer = ai_->GetShotTimer();
	float interval = ai_->GetShotInterval();
	float ratio = 1.0f;
	if (interval > 0.0f) {
		ratio = std::clamp(timer / interval, 0.0f, 1.0f);
	}

	// シリンダーの半径
	float currentRadius = maxRadius_ + (minRadius_ - maxRadius_) * ratio;

	// スケールの適用
	particle.transform.scale.x = currentRadius;
	particle.transform.scale.z = currentRadius;
	particle.transform.scale.y = height_;
}