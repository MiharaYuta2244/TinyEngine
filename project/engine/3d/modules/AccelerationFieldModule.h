#pragma once
#include "ParticleModule.h"
#include "AABB.h"
#include "MathUtility.h"
#include "Collision.h"
#include "MathOperator.h"

// 簡易：ある領域に入ったら加速度を与えるモジュール
class AccelerationFieldModule : public ParticleModule {
public:
    AccelerationFieldModule(const Vector3& accel, const AABB& area)
        : acceleration_(accel), area_(area) {}

    void Update(ParticleState& particle, float deltaTime, EngineContext* ctx) override {
        if (Collision::Intersect(area_, particle.transform.translate)) {
			particle.velocity += acceleration_ * deltaTime;
        }
    }

private:
    Vector3 acceleration_;
    AABB area_;
};