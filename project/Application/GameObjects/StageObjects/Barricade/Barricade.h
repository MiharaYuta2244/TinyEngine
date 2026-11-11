#pragma once
#include "AABB.h"
#include "Collision.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "GameObjects/PlaceableObject.h"
#include "Vector2.h"

/// <summary>
/// 引き剥がすと盾として使えるバリケード
/// </summary>
class Barricade : public PlaceableObject {
public:
	Barricade();

	// 初期化処理
	void Initialize(EngineContext* ctx, const Transform& transform);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw() override;

	// 当たり判定Getter
	AABB GetCollision() const { return collision_; }
	bool IsSolid() const { return CanGrab() || IsPulling(); }

	bool CanGrab() const { return state_ == State::Attached; }
	bool IsPulling() const { return state_ == State::Pulling; }
	bool IsShield() const { return state_ == State::Shield; }
	Vector2 GetGrabOffset(const Vector3& playerPos) const;
	void Grab(const Vector3& playerPos);
	void Pull(const Vector2& moveDir, float deltaTime);
	void PlaceShield(const Vector3& playerPos, const Vector2& forward, float yaw);
	void Release();

	void Throw(const Vector3& playerPos, const Vector2& forward, float yaw);
	bool IsThrown() const { return state_ == State::Thrown; }
	bool HasReachedThrowRange() const { return throwDistanceLeft_ <= 0.0f; }
	bool CheckThrowHit(const AABB& target, float& hitTime) const;
	void LandThrow(float hitTime);

	// ギズモ用
	std::string GetName() const override { return "Barricade(" + std::to_string(id_) + ")"; }

private:
	enum class State { Attached, Pulling, Shield, Thrown, Dropped };

	void UpdateCollision();
	void DropShield();

	State state_ = State::Attached;
	Vector2 pullDirection_{};
	float pullTime_ = 0.0f;
	Transform shieldPose_{{1.0f, 1.25f, 0.18f}, {}, {}};
	Vector2 throwDirection_{};
	float throwDistanceLeft_ = 0.0f;
	Segment throwStep_{};
	AABB collision_{}; // 当たり判定

	// オブジェクト数カウント用
	static int index;
	int id_ = 0;

	// 描画用インスタンス
	std::unique_ptr<ObjectRender> render_;
};
