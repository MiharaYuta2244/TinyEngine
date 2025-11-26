#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "Particle.h"

class ParticleManager {
public:
    void Initialize(EngineContext* ctx) { ctx_ = ctx; }
    // Create and initialize a named Particle instance. Returns pointer.
    Particle* CreateInstance(const std::string& name, Vector3 emitterPos) {
        auto p = std::make_unique<Particle>();
        p->Initialize(ctx_, emitterPos);
        Particle* ptr = p.get();
        instances_.emplace(name, std::move(p));
        return ptr;
    }

    Particle* GetInstance(const std::string& name) {
        auto it = instances_.find(name);
        return (it == instances_.end()) ? nullptr : it->second.get();
    }

    // convenience spawn
    void Spawn(const std::string& name, Vector3 pos, uint32_t count = 1) {
        if (auto p = GetInstance(name)) p->SpawnAt(pos, count);
    }

    void UpdateAll() {
        for (auto& kv : instances_) kv.second->Update();
    }

    void DrawAll() {
        for (auto& kv : instances_) kv.second->Draw();
    }

private:
    EngineContext* ctx_ = nullptr;
    std::unordered_map<std::string, std::unique_ptr<Particle>> instances_;
};
