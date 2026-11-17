#pragma once
#include "AABB.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "ThrowableItem.h"
#include <vector>

namespace TinyEngine {
class Particle;
}

class Molotov : public ThrowableItem {
public:
	Molotov();
	~Molotov() override;

	void Initialize(EngineContext* ctx, const Transform& transform);
	void Update(float deltaTime);
	void Draw() override;
	std::string GetName() const override { return "Molotov(" + std::to_string(GetCollisionId()) + ")"; }

	bool CanGrab() const override { return !inFlight_ && !broken_; }
	void Grab() override {}
	void Throw(const Vector3& direction) override;

	bool IsInFlight() const { return inFlight_; }
	bool CheckThrowHit(const AABB& obstacle, float& time) const;
	void BlockThrow(float time);
	void LandIfGrounded();

	bool IsFireActive() const { return fireTime_ > 0.0f; }
	bool OverlapsFire(const AABB& body) const;
	bool UpdatePlayerExposure(bool inside, float deltaTime);
	void MarkPlayerDamaged() { damagedPlayer_ = true; }
	bool IsFinished() const { return broken_ && !IsFireActive() && particles_.empty(); }

private:
	void Ignite();

	static constexpr float kGroundY = 0.0f;
	static constexpr float kFireRadius = 4.0f;
	static constexpr float kFireDuration = 5.0f;
	static constexpr float kDamageDelay = 1.0f;

	EngineContext* ctx_ = nullptr;
	std::unique_ptr<ObjectRender> render_;
	std::vector<std::unique_ptr<TinyEngine::Particle>> particles_;
	Vector3 previousPos_{};
	Vector3 velocity_{};
	float fireTime_ = 0.0f;
	float playerExposure_ = 0.0f;
	bool damagedPlayer_ = false;
	bool inFlight_ = false;
	bool grounded_ = false;
	bool broken_ = false;
};
