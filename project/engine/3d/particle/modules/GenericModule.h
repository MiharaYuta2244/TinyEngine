#pragma once
#include "EffectData.h"
#include "ParticleModule.h"

/// <summary>
/// パーティクルの挙動の基本となるモジュール
/// </summary>
class GenericModule : public ParticleModule {
public:
	// コンストラクタ
	explicit GenericModule(const ParticleParams& params) : p_(params) {}

	// 初期化関数
	void Initialize(ParticleState& particle, EngineContext* ctx) override;

	// 更新関数
	void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override;

private:
	// パーティクルのパラメータ
	ParticleParams p_;
};