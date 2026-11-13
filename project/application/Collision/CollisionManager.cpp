#include "CollisionManager.h"
#include "Collision.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "GameObjects/Enemy/Weapon/EnemyBombManager.h"
#include "GameObjects/Enemy/Weapon/EnemyBulletManager.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/StageObjects/Stage.h"
#include <algorithm>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

namespace {
// 判定に使う範囲と、押し戻しの対象を保持する
struct Obstacle {
	CollisionObject& object;
	AABB bounds;
	bool blocksPlayer;
	bool blocksEnemy;
};
} // namespace

#pragma region 接触イベントの通知
void CollisionManager::NotifyContact(CollisionObject& first, CollisionPart firstPart, CollisionObject& second, CollisionPart secondPart, const CollisionContext& context) {
	// 判定の順序が逆でも、同じオブジェクトと判定部位の組み合わせとして記録する
	const auto firstCollider = std::pair{first.GetCollisionId(), firstPart};
	const auto secondCollider = std::pair{second.GetCollisionId(), secondPart};
	const auto& low = std::min(firstCollider, secondCollider);
	const auto& high = std::max(firstCollider, secondCollider);
	const Contact contact{low.first, low.second, high.first, high.second};
	if (!currentContacts_.insert(contact).second) {
		return;
	}

	const CollisionEvent firstEvent{second, firstPart, secondPart};
	const CollisionEvent secondEvent{first, secondPart, firstPart};

	// 接触開始は前フレームに接触していなかったペアだけに通知する
	if (!previousContacts_.contains(contact)) {
		first.OnCollisionEnter(firstEvent, context);
		second.OnCollisionEnter(secondEvent, context);
	}

	first.OnCollisionStay(firstEvent, context);
	second.OnCollisionStay(secondEvent, context);
}
#pragma endregion 接触イベントの通知

void CollisionManager::CheckCollisions(Player& player, EnemyManager& enemyManager, EnemyBulletManager& bulletManager, EnemyBombManager& bombManager, Stage& stage, const CollisionContext& context) {
#pragma region 当たり判定に使うオブジェクトの取得
	using enum CollisionPart;
	auto& enemies = enemyManager.GetEnemies();
	const auto& bullets = bulletManager.GetBullets();
	auto& glassManager = *stage.GetGlassManager();
	auto& soapManager = *stage.GetSoapManager();
	const auto& glasses = glassManager.GetObjects();
	const auto& soaps = soapManager.GetObjects();
#pragma endregion 当たり判定に使うオブジェクトの取得

#pragma region 敵の弾がプレイヤーに当たった時の処理
	for (const auto& bullet : bullets) {
		if (bullet->IsCollisionActive() && Collision::Intersect(player.GetBodyCol(), bullet->GetCollision())) {
			NotifyContact(player, Body, *bullet, Body, context);
		}
	}
#pragma endregion 敵の弾がプレイヤーに当たった時の処理

#pragma region プレイヤーの攻撃範囲当たり判定
	player.SetEnableAttack(false);

	// プレイヤーの攻撃範囲と敵
	for (const auto& enemy : enemies) {
		if (!enemy->IsDead() && Collision::Intersect(player.GetAttackCol(), enemy->GetBodyCol())) {
			NotifyContact(player, Attack, *enemy, Body, context);
		}
	}

	// プレイヤーの攻撃範囲とガラス
	if (player.GetIsAttack()) {
		for (const auto& glass : glasses) {
			if (glass->GetIsActive() && Collision::Intersect(player.GetAttackCol(), glass->GetCollision())) {
				NotifyContact(player, Attack, *glass, Body, context);
				break;
			}
		}
	}

	glassManager.RemoveObjectsIf([](const Glass& glass) { return !glass.GetIsActive(); });
#pragma endregion プレイヤーの攻撃範囲当たり判定

#pragma region 障害物の収集
	std::vector<Obstacle> obstacles;
	const auto appendObstacles = [&](const auto& objects, bool blocksEnemy) {
		for (const auto& object : objects) {
			obstacles.push_back({*object, object->GetCollision(), true, blocksEnemy});
		}
	};

	appendObstacles(stage.GetWallManager()->GetObjects(), true);
	for (const auto& barricade : stage.GetBarricadeManager()->GetObjects()) {
		if (barricade->IsSolid()) {
			obstacles.push_back({*barricade, barricade->GetCollision(), true, true});
		}
	}

	// ドアはプレイヤーの接近で開き、閉じている間は敵の移動を妨げる
	for (const auto& door : stage.GetDoorManager()->GetObjects()) {
		if (!door->GetIsOpen()) {
			obstacles.push_back({*door, door->GetCollision(), false, true});
		}
	}

	appendObstacles(glasses, false);
	appendObstacles(stage.GetCageManager()->GetObjects(), false);
#pragma endregion 障害物の収集

#pragma region 障害物からの押し戻しとプレイヤーへの適用
	const auto resolveObstacles = [&](auto& character, bool Obstacle::* blocksCharacter) {
		for (const auto& obstacle : obstacles) {
			if (!(obstacle.*blocksCharacter) || !Collision::Intersect(character.GetBodyCol(), obstacle.bounds)) {
				continue;
			}

			// ノックバック中の敵は、接触通知で死亡することがある
			NotifyContact(character, Body, obstacle.object, Body, context);
			if (character.IsDead()) {
				break;
			}

			const Vector3 displacement = Collision::CalculatePushbackXZ(character.GetBodyCol(), obstacle.bounds);
			if (displacement.x != 0.0f || displacement.z != 0.0f) {
				character.ApplyCollisionDisplacement(displacement);
			}
		}
	};
	resolveObstacles(player, &Obstacle::blocksPlayer);
#pragma endregion 障害物からの押し戻しとプレイヤーへの適用

#pragma region 投げたバリケードの当たり判定
	for (const auto& barricade : stage.GetBarricadeManager()->GetObjects()) {
		if (!barricade->IsThrown()) {
			continue;
		}

		// 移動区間内で最初に当たる障害物または敵を探す
		float closestHit = std::numeric_limits<float>::infinity();
		CollisionObject* hitObject = nullptr;
		const auto test = [&](CollisionObject& object, const AABB& bounds) {
			float hitTime;
			if (barricade->CheckThrowHit(bounds, hitTime) && hitTime < closestHit) {
				closestHit = hitTime;
				hitObject = &object;
			}
		};

		for (const auto& obstacle : obstacles) {
			test(obstacle.object, obstacle.bounds);
		}

		for (const auto& enemy : enemies) {
			if (!enemy->IsDead()) {
				test(*enemy, enemy->GetBodyCol());
			}
		}

		if (hitObject && closestHit <= 1.0f) {
			barricade->LandThrow(closestHit);
			NotifyContact(*barricade, Thrown, *hitObject, Body, context);
		} else if (barricade->HasReachedThrowRange()) {
			barricade->LandThrow(1.0f);
		}
	}
#pragma endregion 投げたバリケードの当たり判定

#pragma region 敵と障害物の当たり判定
	for (const auto& enemy : enemies) {
		if (!enemy->IsDead()) {
			resolveObstacles(*enemy, &Obstacle::blocksEnemy);
		}
	}
#pragma endregion 敵と障害物の当たり判定

#pragma region プレイヤーとゴールの当たり判定
	if (auto* goal = stage.GetGoal(); goal && Collision::Intersect(player.GetBodyCol(), goal->GetCollision())) {
		NotifyContact(player, Body, *goal, Body, context);
	}
#pragma endregion プレイヤーとゴールの当たり判定

#pragma region 敵同士の当たり判定
	// 各ペアを一度だけ調べ、衝突で死亡した敵の判定は打ち切る
	for (auto first = enemies.begin(); first != enemies.end(); ++first) {
		for (auto second = std::next(first); second != enemies.end() && !(*first)->IsDead(); ++second) {
			if (!(*second)->IsDead() && Collision::Intersect((*first)->GetBodyCol(), (*second)->GetBodyCol())) {
				NotifyContact(**first, Body, **second, Body, context);
			}
		}
	}
#pragma endregion 敵同士の当たり判定

#pragma region 敵と敵の弾の当たり判定
	if (player.GetGrabbedEnemy()) {
		for (const auto& enemy : enemies) {
			for (const auto& bullet : bullets) {
				if (!enemy->IsDead() && bullet->IsCollisionActive() && Collision::Intersect(enemy->GetBodyCol(), bullet->GetCollision())) {
					NotifyContact(*enemy, Body, *bullet, Body, context);
				}
			}
		}
	}
#pragma endregion 敵と敵の弾の当たり判定

#pragma region プレイヤーと回復エリアの当たり判定
	for (const auto& healArea : stage.GetHealAreaManager()->GetObjects()) {
		if (healArea->GetIsActive() && Collision::Intersect(player.GetBodyCol(), healArea->GetCollision())) {
			NotifyContact(player, Body, *healArea, Body, context);
		}
	}
#pragma endregion プレイヤーと回復エリアの当たり判定

#pragma region プレイヤーと爆発範囲の当たり判定
	for (const auto& bomb : bombManager.GetBombs()) {
		if (bomb->IsExploded() && Collision::Intersect(player.GetBodyCol(), bomb->GetBombCollision())) {
			NotifyContact(player, Body, *bomb, Explosion, context);
		}
	}
#pragma endregion プレイヤーと爆発範囲の当たり判定

#pragma region 投げた石鹸と障害物の当たり判定
	// 石鹸と障害物の接触は高さを除いて判定する。
	for (const auto& soap : soaps) {
		if (!soap->IsThrown()) {
			continue;
		}

		for (const auto& obstacle : obstacles) {
			if (Collision::IntersectXZ(soap->GetCollision(), obstacle.bounds)) {
				NotifyContact(*soap, Thrown, obstacle.object, Body, context);
				break;
			}
		}
	}
#pragma endregion 投げた石鹸と障害物の当たり判定

#pragma region 石鹸と敵の当たり判定
	for (const auto& soap : soaps) {
		for (const auto& enemy : enemies) {
			if (enemy->IsDead() || enemy->IsDown()) {
				continue;
			}

			// 同じ敵に対しては、泡よりも石鹸本体の直撃を優先する
			if (soap->IsDangerous() && Collision::Intersect(enemy->GetBodyCol(), soap->GetCollision())) {
				NotifyContact(*soap, Thrown, *enemy, Body, context);
				continue;
			}

			// 泡と敵の当たり判定
			if (soap->GetIsBubbleActive() && Collision::Intersect(enemy->GetBodyCol(), soap->GetBubbleArea())) {
				NotifyContact(*soap, Bubble, *enemy, Body, context);
				break;
			}
		}
	}
	soapManager.RemoveObjectsIf([](const Soap& soap) { return soap.IsConsumed(); });
#pragma endregion 石鹸と敵の当たり判定

#pragma region 次フレーム用の接触履歴の更新
	previousContacts_.swap(currentContacts_);
	currentContacts_.clear();
#pragma endregion 次フレーム用の接触履歴の更新
}
