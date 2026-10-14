#pragma once
#include "EffectData.h"
#include "Particle.h"
#include <memory>
#include <optional>
#include <vector>

namespace EffectBuilder {

/// <summary>
/// 生成時に渡せる実行時オプション
/// </summary>
struct SpawnOptions {
	float sizeScale = 1.0f;           // startSizeを一括で拡縮(爆発半径に合わせる等)
	std::optional<Vector3> extent;    // 発生範囲の上書き(ガラス破壊等)
	std::optional<Vector3> direction; // useSpawnDirection=trueのエミッターに使う方向
};

// エフェクトの生成
std::vector<std::unique_ptr<TinyEngine::Particle>> Build(EngineContext* ctx, const EffectData& data, const Vector3& pos, const SpawnOptions& opt = {});

// コンテナに追加する
template<class Container> void Spawn(EngineContext* ctx, const EffectData& data, const Vector3& pos, Container& out, const SpawnOptions& opt = {}) {
	for (auto& p : Build(ctx, data, pos, opt)) {
		out.push_back(std::move(p));
	}
}

// エディターのコンボ用 ("Generic" + モジュール名)
const std::vector<std::string>& GetModuleTypeNames();
} // namespace EffectBuilder