#pragma once
#include "EaseType.h"
#include "ParticleTypes.h"
#include "Vector3.h"
#include "Vector4.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

struct FloatRange {
	float min = 0.0f;
	float max = 0.0f;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(FloatRange, min, max)

/// <summary>
/// エミッターの形状
/// </summary>
enum class EmitShape { Cone, CircleXZ };
NLOHMANN_JSON_SERIALIZE_ENUM(
    EmitShape, {
                   {EmitShape::Cone,     "Cone"    },
                   {EmitShape::CircleXZ, "CircleXZ"},
})

/// <summary>
/// GenericModuleが使うパラメータ
/// </summary>
struct ParticleParams {
	// 寿命
	FloatRange life = {0.3f, 0.6f};

	// 初速
	EmitShape shape = EmitShape::Cone;      // spreadAngle=180で全方向(球)
	Vector3 direction = {0.0f, 1.0f, 0.0f}; // Coneの中心方向
	float spreadAngle = 180.0f;             // Coneの半角
	FloatRange speed = {1.0f, 3.0f};
	FloatRange yVelocity = {0.0f, 0.0f}; // CircleXZのときのY速度
	bool useSpawnDirection = false;      // trueなら生成時に渡された方向をdirectionに使う(マズルフラッシュ等)

	// 物理
	Vector3 gravity = {0.0f, 0.0f, 0.0f};
	float damping = 1.0f; // 1/60秒あたりの速度保持率

	// サイズ
	FloatRange startSize = {0.2f, 0.4f};
	Vector3 sizeAxis = {1.0f, 1.0f, 1.0f}; // 軸ごとの倍率
	float endScale = 0.0f;                 // 寿命終了時の倍率(1で変化なし)
	EaseType sizeEase = EaseType::EASELINEAR;

	// 回転
	Vector3 initialRotateDeg = {90.0f, 0.0f, 0.0f}; // 90,0,0 で地面に水平
	FloatRange spinDeg = {0.0f, 0.0f};              // Z軸回転(度/秒)

	// 色
	Vector4 colorStartA = {1.0f, 1.0f, 1.0f, 1.0f};
	Vector4 colorStartB = {1.0f, 1.0f, 1.0f, 1.0f};
	Vector4 colorEnd = {1.0f, 1.0f, 1.0f, 0.0f};
	EaseType colorEase = EaseType::EASELINEAR;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    ParticleParams, life, shape, direction, spreadAngle, speed, yVelocity, useSpawnDirection, gravity, damping, startSize, sizeAxis, endScale, sizeEase, initialRotateDeg, spinDeg, colorStartA,
    colorStartB, colorEnd, colorEase)

/// <summary>
/// Particle1個分
/// </summary>
struct EmitterData {
	std::string name = "Emitter";
	std::string texture = "white.png";
	TinyEngine::ParticleMeshType mesh = TinyEngine::ParticleMeshType::Square;
	TinyEngine::ParticleBlendMode blend = TinyEngine::ParticleBlendMode::Additive;

	// 発生設定
	bool burst = true;     // trueのときcount個を1回だけ発生
	bool loop = false;     // burst=falseのとき
	float duration = 0.1f; // burst=false && loop=falseのとき発生を続ける時間
	uint32_t count = 10;
	float frequency = 0.1f; // burst=falseのときの発生間隔
	Vector3 offset = {0.0f, 0.0f, 0.0f};
	Vector3 extent = {0.0f, 0.0f, 0.0f}; // 発生範囲

	std::string moduleType = "Generic"; // 既存モジュール名
	ParticleParams params;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(EmitterData, name, texture, mesh, blend, burst, loop, duration, count, frequency, offset, extent, moduleType, params)

/// <summary>
/// エフェクトデータ
/// </summary>
struct EffectData {
	std::string name;
	std::vector<EmitterData> emitters;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(EffectData, name, emitters)