#pragma once
#include "CollisionEvent.h"
#include <set>
#include <tuple>

class Player;
class EnemyManager;
class EnemyBulletManager;
class EnemyBombManager;
class Stage;

/// <summary>
/// 当たり判定管理クラス
/// </summary>
class CollisionManager {
public:
	// 全てのゲームオブジェクトの当たり判定の処理を回す
	void CheckCollisions(
	    Player& player, EnemyManager& enemyManager, EnemyBulletManager& bulletManager, EnemyBombManager& bombManager,
	    Stage& stage, const CollisionContext& context);

private:
	using Contact = std::tuple<std::uint64_t, CollisionPart, std::uint64_t, CollisionPart>;

	void NotifyContact(
	    CollisionObject& first, CollisionPart firstPart, CollisionObject& second, CollisionPart secondPart,
	    const CollisionContext& context);

	std::set<Contact> previousContacts_;
	std::set<Contact> currentContacts_;
};