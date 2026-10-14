#pragma once
#include <nlohmann/json.hpp>

namespace TinyEngine {

/// <summary>
/// パーティクルのメッシュタイプ
/// </summary>
enum class ParticleMeshType {
	Square,
	Ring,
	Cylinder,
};

/// <summary>
/// パーティクルのブレンドモード
/// </summary>
enum class ParticleBlendMode {
	Additive,
	Alpha,
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    ParticleMeshType, {
                          {ParticleMeshType::Square,   "Square"  },
                          {ParticleMeshType::Ring,     "Ring"    },
                          {ParticleMeshType::Cylinder, "Cylinder"},
})

NLOHMANN_JSON_SERIALIZE_ENUM(
    ParticleBlendMode, {
                           {ParticleBlendMode::Additive, "Additive"},
                           {ParticleBlendMode::Alpha,    "Alpha"   },
})
} // namespace TinyEngine