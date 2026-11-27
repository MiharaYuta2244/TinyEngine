#pragma once
#include "ParticleModule.h"

class HitRingModule : public ParticleModule {
public:
	HitRingModule(Vector4 color, float finalSize) : color_(color), finalSize_(finalSize) {}

	void Initialize(ParticleState& particle, EngineContext* ctx) override;

	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
	float baseSize_ = 0.1f;
	float finalSize_;
	Vector4 color_;
};