#pragma once
#include "Vector3.h"
#include <atomic>
#include <cstdint>
#include <functional>

class Camera;
class CollisionObject;
class Enemy;
class Glass;

// 当たり判定用のタグ
enum class CollisionTag {
	None, Player, Enemy, EnemyBullet, EnemyBomb, Wall, Barricade, Door, Glass, Cage, Goal, HealArea, Soap,
};

// 同じオブジェクトの本体と判定範囲を別々の接触として扱う
enum class CollisionPart { Body, Attack, Explosion, Bubble, Thrown };

struct CollisionEvent {
	CollisionObject& other;
	CollisionPart selfPart;
	CollisionPart otherPart;
};

struct CollisionContext {
	Camera& camera;
	std::function<void(Enemy&)> onEnemyDeath;
	std::function<void(Glass&, const Vector3&)> onGlassBreak;
	std::function<void()> onPlayerHit;
};

class CollisionObject {
public:
	CollisionObject() = default;
	CollisionObject(const CollisionObject&) = delete;
	CollisionObject& operator=(const CollisionObject&) = delete;
	virtual ~CollisionObject() = default;

	std::uint64_t GetCollisionId() const { return collisionId_; }
	virtual CollisionTag GetTag() const { return CollisionTag::None; }
	bool CompareTag(CollisionTag tag) const { return GetTag() == tag; }

	// 接触が始まったフレームに一度だけ呼ばれる
	virtual void OnCollisionEnter(const CollisionEvent&, const CollisionContext&) {}

	// 接触が検出されたフレームごとに呼ばれる。初回も含む
	virtual void OnCollisionStay(const CollisionEvent&, const CollisionContext&) {}

private:
	inline static std::atomic<std::uint64_t> nextCollisionId_{1};
	const std::uint64_t collisionId_ = nextCollisionId_++;
};
