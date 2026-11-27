#pragma once
#include "ParticleModule.h"
#include "Random.h"

/// <summary>
/// 木の破片が飛び散るパーティクルのモジュール
/// </summary>
class WoodShardModule : public ParticleModule {
public:
	void Initialize(ParticleState& particle, EngineContext* ctx) override;
	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
	float baseSize_ = 0.15f;
	float gravity_ = -25.0f;
	float spinSpeed_ = 10.0f;
};