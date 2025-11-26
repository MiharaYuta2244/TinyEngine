#pragma once
#include "ParticleModule.h"
#include "Random.h"

// 埃（Dust）モジュール
class DustModule : public ParticleModule {
public:
    // コンストラクタ：寿命範囲、初速の大きさ、サイズ範囲、重力スケール（正は下向き）
    DustModule(float lifeMin = 0.4f, float lifeMax = 0.9f,
               float baseSpeed = 0.08f,
               float sizeMin = 0.15f, float sizeMax = 0.35f,
               float gravityScale = -9.8f * 0.05f)
        : lifeMin_(lifeMin),
          lifeMax_(lifeMax),
          baseSpeed_(baseSpeed),
          sizeMin_(sizeMin),
          sizeMax_(sizeMax),
          gravityScale_(gravityScale) {}

    void Initialize(ParticleState& particle, EngineContext* ctx) override;
    void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
    float lifeMin_;
    float lifeMax_;
    float baseSpeed_;
    float sizeMin_;
    float sizeMax_;
    float gravityScale_;
    const float damping_ = 0.98f; // 速度減衰
};