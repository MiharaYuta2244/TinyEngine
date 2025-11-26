#pragma once
#include "ParticleModule.h"

// 衝撃波：発生時に小さく始まり時間経過でスケールを拡大してフェードアウトする
class ShockWaveModule : public ParticleModule {
public:
    ShockWaveModule(float life = 0.6f, float startScale = 0.2f, float endScale = 3.0f)
        : life_(life), startScale_(startScale), endScale_(endScale) {}

    void Initialize(ParticleState& particle, EngineContext* ctx) override;
    void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
    float life_;
    float startScale_;
    float endScale_;
};