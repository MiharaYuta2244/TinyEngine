#pragma once
#include "ParticleModule.h"
#include "Random.h"

/// <summary>
/// ガラスの破片が飛び散るパーティクルのモジュール
/// </summary>
class GlassShardModule : public ParticleModule {
public:
	void Initialize(ParticleState& particle, EngineContext* ctx) override;
	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
	float baseSize_ = 0.12f;
};