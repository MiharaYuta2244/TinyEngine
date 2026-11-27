#include "EffectBuilder.h"
#include "GenericModule.h"
#include "MathOperator.h"
#include "ParticleModuleHeader.h"
#include <functional>
#include <unordered_map>

using namespace TinyEngine;

namespace {
using Factory = std::function<std::unique_ptr<ParticleModule>()>;

// 引数なしで作れるモジュール
const std::unordered_map<std::string, Factory>& LegacyFactories() {
	static const std::unordered_map<std::string, Factory> table = {
	    {"Dust",        [] { return std::make_unique<DustModule>(); }       },
	    {"RadialRing",  [] { return std::make_unique<RadialRingModule>(); } },
	    {"Rising",      [] { return std::make_unique<RisingModule>(); }     },
	    {"ShockWave",   [] { return std::make_unique<ShockWaveModule>(); }  },
	    {"DustStep",    [] { return std::make_unique<DustStepModule>(); }   },
	    {"HealCross",   [] { return std::make_unique<HealCrossModule>(); }  },
	    {"HealSphere",  [] { return std::make_unique<HealSphereModule>(); } },
	    {"HealLine",    [] { return std::make_unique<HealLineModule>(); }   },
	    {"HitFlash",    [] { return std::make_unique<HitFlashModule>(); }   },
	    {"HitSpark",    [] { return std::make_unique<HitSparkModule>(); }   },
	    {"MuzzleFlash", [] { return std::make_unique<MuzzleFlashModule>(); }},
	    {"GlassShard",  [] { return std::make_unique<GlassShardModule>(); } },
	    {"WoodShard",   [] { return std::make_unique<WoodShardModule>(); }  },
	};
	return table;
}

std::unique_ptr<ParticleModule> CreateModule(const EmitterData& e) {
	if (e.moduleType == "Generic") {
		return std::make_unique<GenericModule>(e.params);
	}

	auto it = LegacyFactories().find(e.moduleType);
	if (it != LegacyFactories().end()) {
		return it->second();
	}

	return std::make_unique<GenericModule>(e.params); // 不明な名前はGenericにフォールバック
}
} // namespace

std::vector<std::unique_ptr<Particle>> EffectBuilder::Build(EngineContext* ctx, const EffectData& data, const Vector3& pos, const SpawnOptions& opt) {
	std::vector<std::unique_ptr<Particle>> result;

	for (const EmitterData& src : data.emitters) {
		// 実行時オプションを反映するためコピーして使う
		EmitterData e = src;
		e.params.startSize.min *= opt.sizeScale;
		e.params.startSize.max *= opt.sizeScale;
		if (opt.extent) {
			e.extent = *opt.extent;
		}
		if (e.params.useSpawnDirection && opt.direction) {
			e.params.direction = *opt.direction;
		}

		Particle::Emitter emitter{};
		emitter.transform.scale = e.extent;

		auto particle = std::make_unique<Particle>();
		particle->Initialize(ctx, pos + e.offset, e.texture, CreateModule(e), &emitter, e.mesh);
		particle->SetBlendMode(e.blend);
		particle->SetBurst(e.burst);
		particle->SetEmitMode(e.loop, e.duration);
		particle->SetEmitterParam(e.count, e.burst ? 0.0f : e.frequency);
		result.push_back(std::move(particle));
	}
	return result;
}

const std::vector<std::string>& EffectBuilder::GetModuleTypeNames() {
	static const std::vector<std::string> names = [] {
		std::vector<std::string> v = {"Generic"};
		for (auto& [name, _] : LegacyFactories()) {
			v.push_back(name);
		}
		return v;
	}();
	return names;
}