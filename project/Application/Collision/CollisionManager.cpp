#include "CollisionManager.h"
#include "Collision.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "GameObjects/Enemy/Weapon/EnemyBomb.h"
#include "GameObjects/Enemy/Weapon/EnemyBullet.h"
#include "GameObjects/Enemy/Weapon/EnemyWeaponManager.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/StageObjects/Stage.h"
#include "MathOperator.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

namespace {
// キャラクターモデルの足元から頭部までの高さ（モデル空間）
constexpr float kEnemyModelHeight = 8.6f;
constexpr float kPlayerModelHeight = 2.45f;

// 判定に使う範囲と、押し戻しの対象を保持する
struct Obstacle {
	IGameObject& object;
	AABB bounds;
	bool blocksPlayer;
	bool blocksEnemy;
};

std::vector<Obstacle> CollectObstacles(Stage& stage) {
	std::vector<Obstacle> obstacles;
	const auto appendObstacles = [&](const auto& objects, bool blocksEnemy) {
		for (const auto& object : objects) {
			obstacles.push_back({*object, object->GetCollision(), true, blocksEnemy});
		}
	};

	appendObstacles(stage.GetWallManager()->GetObjects(), true);
	appendObstacles(stage.GetDecorationManager()->GetObjects(), true);
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

	appendObstacles(stage.GetGlassManager()->GetObjects(), false);
	appendObstacles(stage.GetCageManager()->GetObjects(), false);
	return obstacles;
}
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

void CollisionManager::CheckCollisions(Player& player, EnemyManager& enemyManager, const EnemyWeaponManager& enemyWeapons, Stage& stage, const CollisionContext& context, float deltaTime) {
	// 前フレームのビームも含め、接触履歴を更新する
	previousContacts_.swap(currentContacts_);
	currentContacts_.clear();

#pragma region 当たり判定に使うオブジェクトの取得
	using enum CollisionPart;
	auto& enemies = enemyManager.GetEnemies();
	auto& glassManager = *stage.GetGlassManager();
	auto& soapManager = *stage.GetSoapManager();
	const auto& glasses = glassManager.GetObjects();
	const auto& soaps = soapManager.GetObjects();
#pragma endregion 当たり判定に使うオブジェクトの取得

#pragma region プレイヤーの攻撃範囲当たり判定
	player.SetEnableAttack(false);

	// プレイヤーの攻撃範囲と敵
	for (const auto& enemy : enemies) {
		if (!enemy->IsDead() && Collision::Intersect(player.GetAttackCol(), enemy->GetBodyCol())) {
			NotifyContact(player, Attack, *enemy, Body, context);
		}
	}

	if (player.GetIsAttack() && !player.IsDead()) {
		// 攻撃範囲内で最も近い宝箱を1つ開ける
		TreasureChest* target = nullptr;
		float closestDistance = std::numeric_limits<float>::infinity();
		for (const auto& chest : stage.GetTreasureChestManager()->GetObjects()) {
			if (!chest->IsOpen() && Collision::Intersect(player.GetAttackCol(), chest->GetCollision())) {
				const float distance = MathUtility::LengthSquared(chest->GetCollision().center - player.GetPosition());
				if (distance < closestDistance) {
					closestDistance = distance;
					target = chest.get();
				}
			}
		}

		if (target) {
			NotifyContact(player, Attack, *target, Body, context);
		} else {
			for (const auto& glass : glasses) {
				if (glass->GetIsActive() && Collision::Intersect(player.GetAttackCol(), glass->GetCollision())) {
					NotifyContact(player, Attack, *glass, Body, context);
					break;
				}
			}
		}
	}

	glassManager.RemoveObjectsIf([](const Glass& glass) { return !glass.GetIsActive(); });
#pragma endregion プレイヤーの攻撃範囲当たり判定

#pragma region 障害物の収集
	const auto obstacles = CollectObstacles(stage);
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

#pragma region プレイヤーと爆発範囲の当たり判定
	for (const auto& bomb : enemyWeapons.GetBombs()) {
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
			if (enemy->IsDead() || enemy->IsDown() || enemy->IsBurning()) {
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

#pragma region 火炎瓶の着地と炎の当たり判定
	auto& molotovManager = *stage.GetMolotovManager();
	for (const auto& molotov : molotovManager.GetObjects()) {
		const bool wasBurning = molotov->IsFireActive();
		if (molotov->IsInFlight()) {
			float closestHit = std::numeric_limits<float>::infinity();
			for (const auto& obstacle : obstacles) {
				float hitTime;
				AABB bounds = obstacle.bounds;
				// 壁などの地上オブジェクトは既存の当たり判定がXZ平面にある
				if (bounds.min.y == bounds.max.y) {
					const auto& transform = obstacle.object.GetTransform();
					bounds.max.y += std::abs(transform.scale.y);
				}
				if (molotov->CheckThrowHit(bounds, hitTime)) {
					closestHit = std::min(closestHit, hitTime);
				}
			}
			if (closestHit <= 1.0f) {
				molotov->BlockThrow(closestHit);
			}
			molotov->LandIfGrounded();
		}

		if (!molotov->IsFireActive()) {
			continue;
		}
		for (const auto& enemy : enemies) {
			if (!enemy->IsDead() && molotov->OverlapsFire(enemy->GetBodyCol())) {
				enemy->Ignite();
			}
		}

		const bool inside = !player.IsDead() && molotov->OverlapsFire(player.GetBodyCol());
		if (molotov->UpdatePlayerExposure(inside, wasBurning ? deltaTime : 0.0f)) {
			const float previousHP = player.GetCurrentHP();
			player.Damage(1);
			if (player.GetCurrentHP() < previousHP) {
				molotov->MarkPlayerDamaged();
				context.onPlayerHit();
			}
		}
	}
	molotovManager.RemoveObjectsIf([](const Molotov& molotov) { return molotov.IsFinished(); });
#pragma endregion 火炎瓶の着地と炎の当たり判定

}

void CollisionManager::CheckEnemyBeams(
    Player& player, EnemyManager& enemyManager, const EnemyWeaponManager& enemyWeapons, Stage& stage, const CollisionContext& context, float deltaTime) {
	const auto& bullets = enemyWeapons.GetBullets();
	if (bullets.empty()) {
		return;
	}

	const auto obstacles = CollectObstacles(stage);
	for (const auto& bullet : bullets) {
		const float distance = bullet->GetGrowthDistance(deltaTime);
		if (distance <= 0.0f) {
			continue;
		}

		// 先端が今回進む区間から、一番手前の接触だけを選ぶ
		const Segment segment{bullet->GetTip(), bullet->GetDirection() * distance};
		float closestHit = std::numeric_limits<float>::infinity();
		CollisionObject* hitObject = nullptr;
		const auto recordHit = [&](CollisionObject& object, float hitTime) {
			if (hitTime < closestHit) {
				closestHit = hitTime;
				hitObject = &object;
			}
		};
		const auto test = [&](CollisionObject& object, const auto& bounds) {
			float hitTime;
			if (Collision::Intersect(segment, bounds, hitTime)) {
				recordHit(object, hitTime);
			}
		};
		const auto testCharacter = [&](auto& character, float modelHeight) {
			// 移動用の判定は足元にあるため、ビームには胴体と頭部の高さも含める。
			AABB bounds = character.GetBodyCol();
			const auto& pose = character.GetTransform();
			bounds.max.y = std::max(bounds.max.y, pose.translate.y + modelHeight * std::abs(pose.scale.y));
			test(character, bounds);
		};

		for (const auto& obstacle : obstacles) {
			// バリケードは構えている盾も含めて、現在の描画姿勢で判定する
			if (obstacle.object.CompareTag(CollisionTag::Barricade)) {
				continue;
			}
			AABB bounds = obstacle.bounds;
			if (bounds.min.y == bounds.max.y) {
				const float halfHeight = std::abs(obstacle.object.GetTransform().scale.y);
				bounds.min.y -= halfHeight;
				bounds.max.y += halfHeight;
			}
			test(obstacle.object, bounds);
		}

		for (const auto& barricade : stage.GetBarricadeManager()->GetObjects()) {
			float hitTime;
			if (barricade->CheckBeamHit(segment, hitTime)) {
				recordHit(*barricade, hitTime);
			}
		}

		for (const auto& chest : stage.GetTreasureChestManager()->GetObjects()) {
			test(*chest, chest->GetCollision());
		}

		for (const auto& enemy : enemyManager.GetEnemies()) {
			if (!enemy->IsDead() && enemy->GetCollisionId() != bullet->GetShooterId()) {
				testCharacter(*enemy, kEnemyModelHeight);
			}
		}

		if (!player.IsDead()) {
			testCharacter(player, kPlayerModelHeight);
		}

		bullet->Extend(hitObject ? distance * closestHit : distance, hitObject != nullptr);
		if (hitObject == &player) {
			NotifyContact(player, CollisionPart::Body, *bullet, CollisionPart::Body, context);
		} else if (hitObject && hitObject == player.GetGrabbedEnemy()) {
			NotifyContact(*hitObject, CollisionPart::Body, *bullet, CollisionPart::Body, context);
			player.ReleaseGrabbedEnemy();
		}
	}
}
