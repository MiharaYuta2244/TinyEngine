#include "WoodShardModule.h"
#include <algorithm>
#include <cmath>
#include <numbers>

void WoodShardModule::Initialize(ParticleState& particle, EngineContext* /*ctx*/) {
	// 放射状に飛ばしつつ、少し上へ跳ね上げる
	float angle = RandomUtils::RangeFloat(0.0f, 2.0f * std::numbers::pi_v<float>);
	float speed = RandomUtils::RangeFloat(4.0f, 10.0f);
	particle.velocity = {std::cos(angle) * speed, RandomUtils::RangeFloat(5.0f, 10.0f), std::sin(angle) * speed};

	// 明るめの茶色
	float t = RandomUtils::RangeFloat(0.0f, 1.0f);
	particle.color = {0.65f + 0.2f * t, 0.4f + 0.15f * t, 0.15f, 1.0f};

	particle.lifeTime = RandomUtils::RangeFloat(0.5f, 0.9f);
	particle.currentTime = 0.0f;

	// 細長い板切れ
	float w = baseSize_ * RandomUtils::RangeFloat(0.6f, 1.2f);
	particle.transform.scale = {w, w * RandomUtils::RangeFloat(2.0f, 3.5f), 1.0f};
	particle.transform.rotate = {std::numbers::pi_v<float> / 2.0f, RandomUtils::RangeFloat(0.0f, std::numbers::pi_v<float>), 0.0f};
}

void WoodShardModule::Update(ParticleState& particle, float deltaTime, EngineContext* /*ctx*/) {
	// 重力 + 空気抵抗
	particle.velocity.y += gravity_ * deltaTime;
	float damp = std::pow(0.96f, deltaTime * 60.0f);
	particle.velocity.x *= damp;
	particle.velocity.z *= damp;

	// 地面より下に行かない
	if (particle.transform.translate.y < 0.1f && particle.velocity.y < 0.0f) {
		particle.velocity.y = 0.0f;
		particle.velocity.x *= 0.8f;
		particle.velocity.z *= 0.8f;
	}

	// 回転させて破片らしく
	particle.transform.rotate.y += spinSpeed_ * deltaTime;

	// 後半でフェードアウト
	float t = std::clamp(particle.currentTime / particle.lifeTime, 0.0f, 1.0f);
	particle.color.w = 1.0f - t * t;
}