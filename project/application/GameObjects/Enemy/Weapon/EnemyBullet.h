#pragma once
#include "AnimationBundle.h"
#include "Collision/CollisionEvent.h"
#include "GameObjects/ObjectRender/ObjectRender.h"

/// <summary>
/// 銃口を起点に、命中するか最大射程に達するまで伸びる敵のビーム
/// </summary>
class EnemyBullet : public CollisionObject {
public:
	CollisionTag GetTag() const override { return CollisionTag::EnemyBullet; }

	void Initialize(EngineContext* ctx, const Vector3& muzzle, const Vector3& direction, std::uint64_t shooterId);
	void Update(float deltaTime);
	void Draw();

	bool IsDead() const { return deathTimer_ >= kLifeTime; }
	const Vector3& GetDirection() const { return direction_; }
	std::uint64_t GetShooterId() const { return shooterId_; }
	Vector3 GetTip() const;
	float GetGrowthDistance(float deltaTime) const;
	void Extend(float distance, bool hit);

private:
	static constexpr float kLifeTime = 0.5f;
	static constexpr float kMaxLength = 30.0f;
	static constexpr float kGrowthSpeed = 180.0f;

	Transform transform_{};
	std::unique_ptr<ObjectRender> render_;
	Vector3 muzzle_{};
	Vector3 direction_{};
	std::uint64_t shooterId_ = 0;
	float length_ = 0.0f;
	float deathTimer_ = 0.0f;
	bool hasHit_ = false;
	AnimationBundle<Vector2> scaleXYAnim_;
};
