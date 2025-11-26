#include "ShockWaveModule.h"
#include "Random.h"
#include <algorithm>

void ShockWaveModule::Initialize(ParticleState& particle, EngineContext* /*ctx*/) {
    particle.lifeTime = life_;
    particle.currentTime = 0.0f;
    // 初期は小さめ、中央に出る想定
    particle.transform.scale = {startScale_, startScale_, startScale_};
    particle.velocity = {0.0f, 0.0f, 0.0f};
    // 色は白っぽくしておく（alpha は Update で調整）
    particle.color = {1.0f, 0.95f, 0.8f, 1.0f};
    // 衝撃波は通常テクスチャを差し替えたいならここで指定
    // particle.textureFilePath = "resources/shockwave.png";
}

void ShockWaveModule::Update(ParticleState& particle, float /*deltaTime*/, EngineContext* /*ctx*/) {
    if (particle.lifeTime <= 0.0f) return;
    float t = particle.currentTime / particle.lifeTime;
    t = std::clamp(t, 0.0f, 1.0f);
    // スケールを線形補間で拡大
    float s = startScale_ + (endScale_ - startScale_) * t;
    particle.transform.scale = {s, s, s};
    // フェードアウト
    particle.color.w = 1.0f - t;
}