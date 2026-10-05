#pragma once
#include "EngineContext.h"
#include "GameObjects/Enemy/EnemyType.h"
#include <memory>
#include <list>

class EnemyBulletManager;
class EnemyBombManager;

namespace TinyEngine {
class Particle;
}

struct ShotContext {
	EngineContext* engine;
	Vector3 origin;   // 敵の座標
	Vector3 toTarget; // 発射方向
	Vector3 lastKnownPlayerPos;
	float bulletMargin;
	EnemyBulletManager* bullets;
	EnemyBombManager* bombs;
};

/// <summary>
/// 敵の挙動の基底クラス
/// </summary>
class IEnemyBehavior {
public:
	virtual ~IEnemyBehavior() = default;

	// 敵の種類Getter
	virtual EnemyType GetType() const = 0;

	// 最大HPGetter
	virtual int GetMaxHP() const = 0;

	// 色のGetter
	virtual Vector4 GetColor() const = 0;

	// 弾の発射インターバルGetter
	virtual float GetShotInterval() const { return 1.5f; }

	// 弾の発射処理
	virtual void Shot(const ShotContext& c) = 0;

	// 死亡時
	virtual void OnDeath(EngineContext* ctx, const Vector3& pos, std::list<std::unique_ptr<TinyEngine::Particle>>& deathEffect) {}
	virtual float GetDeathBlastRadius() const { return 0.0f; } 
};