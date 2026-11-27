#include "GlassShardModule.h"
#include <cmath>
#include <numbers>

void GlassShardModule::Initialize(ParticleState& particle, EngineContext* /*ctx*/) {
	// XZ平面に放射状に飛ばす
	float angle = RandomUtils::RangeFloat(0.0f, 2.0f * std::numbers::pi_v<float>);
	float speed = RandomUtils::RangeFloat(6.0f, 16.0f);
	particle.velocity = {std::cos(angle) * speed, RandomUtils::RangeFloat(1.0f, 4.0f), std::sin(angle) * speed};

	// 水色から白のランダムな色
	float t = RandomUtils::RangeFloat(0.0f, 1.0f);
	particle.color = {0.6f + 0.4f * t, 0.85f + 0.15f * t, 1.0f, 1.0f};

	particle.lifeTime = RandomUtils::RangeFloat(0.5f, 0.9f);
	particle.currentTime = 0.0f;

	// 細長い破片にして、進行方向へ向ける
	float w = baseSize_ * RandomUtils::RangeFloat(0.6f, 1.2f);
	particle.transform.scale = {w, w * RandomUtils::RangeFloat(2.0f, 4.0f), 1.0f};
	particle.transform.rotate = {std::numbers::pi_v<float> / 2.0f, std::atan2(particle.velocity.x, particle.velocity.z), 0.0f};
}

void GlassShardModule::Update(ParticleState& particle, float deltaTime, EngineContext* /*ctx*/) {
	// 減速
	float damp = std::pow(0.9f, deltaTime * 60.0f);
	particle.velocity.x *= damp;
	particle.velocity.y *= damp;
	particle.velocity.z *= damp;

	// 縮小してフェードアウト
	float shrink = std::pow(0.93f, deltaTime * 60.0f);
	particle.transform.scale.x *= shrink;
	particle.transform.scale.y *= shrink;

	particle.color.w = std::clamp(1.0f - particle.currentTime / particle.lifeTime, 0.0f, 1.0f);
}