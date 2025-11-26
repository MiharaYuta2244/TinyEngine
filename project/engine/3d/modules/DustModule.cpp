#include "DustModule.h"
#include "MathOperator.h" // Vector ops が必要な場合に備えインクルード

void DustModule::Initialize(ParticleState& particle, EngineContext* /*ctx*/) {
    // 寿命
    particle.lifeTime = RandomUtils::RangeFloat(lifeMin_, lifeMax_);
    particle.currentTime = 0.0f;

    // サイズ（スケール）
    float s = RandomUtils::RangeFloat(sizeMin_, sizeMax_);
    particle.transform.scale = {s, s, s};

    // 色：薄めの灰色〜ベージュをランダムで
    float g = RandomUtils::RangeFloat(0.6f, 0.85f);
    float r = g * RandomUtils::RangeFloat(0.9f, 1.0f);
    float b = g * RandomUtils::RangeFloat(0.9f, 1.0f);
    particle.color = {r, g, b, 1.0f};

    // 初速：プレイヤー足元の埃として横に散る + 少し上向き
    float angle = RandomUtils::RangeFloat(0.0f, 2.0f * 3.14159265f);
    float speedR = RandomUtils::RangeFloat(0.0f, baseSpeed_);
    particle.velocity.x = std::cos(angle) * speedR;
    particle.velocity.z = std::sin(angle) * speedR;
    particle.velocity.y = RandomUtils::RangeFloat(baseSpeed_ * 0.2f, baseSpeed_ * 0.6f);
}

void DustModule::Update(ParticleState& particle, float deltaTime, EngineContext* /*ctx*/) {
    // 重力を軽く適用（負値なら下向き）
    particle.velocity.y += gravityScale_ * deltaTime;

    // 速度減衰
    particle.velocity.x *= std::pow(damping_, deltaTime * 60.0f);
    particle.velocity.y *= std::pow(damping_, deltaTime * 60.0f);
    particle.velocity.z *= std::pow(damping_, deltaTime * 60.0f);

    // 透明度を寿命比でフェード（PrepareRender 側で上書きされていなければ描画に反映される）
    if (particle.lifeTime > 0.0f) {
        float t = particle.currentTime / particle.lifeTime;
        particle.color.w = std::clamp(1.0f - t, 0.0f, 1.0f);
    }
}