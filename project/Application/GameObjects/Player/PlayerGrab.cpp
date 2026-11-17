#include "PlayerGrab.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "GameObjects/Item/Soap.h"
#include "GameObjects/Item/Molotov.h"
#include "GameObjects/StageObjects/Barricade/Barricade.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include <cfloat>
#include <algorithm>

PlayerGrab::Result PlayerGrab::Update(
    const Input& input, const Context& ctx, EnemyManager* enemyManager, const std::list<std::unique_ptr<Soap>>& soaps,
    const std::list<std::unique_ptr<Molotov>>& molotovs,
    const std::list<std::unique_ptr<Barricade>>& barricades) {

	Result result;

	// エディターで削除されたアイテムや、炎上した敵を手放す。
	const auto containsHeldItem = [this](const auto& items) {
		return std::any_of(items.begin(), items.end(), [this](const auto& item) { return item.get() == heldItem_; });
	};
	if (heldItem_ && !containsHeldItem(soaps) && !containsHeldItem(molotovs)) {
		heldItem_ = nullptr;
	}
	if (heldEnemy_ && heldEnemy_->IsBurning()) {
		ReleaseEnemy();
	}


	if (heldBarricade_) {
		if (heldBarricade_->IsShield() && input.attackPressed) {
			heldBarricade_->Throw(ctx.position, ctx.forward, ctx.rotate.y);
			heldBarricade_ = nullptr;
			result.attacked = true;
			result.stopMovement = true;
		} else if (input.grabHeld) {
			if (heldBarricade_->IsPulling()) {
				heldBarricade_->Pull(input.moveDir, ctx.deltaTime);
			}
		} else {
			ReleaseBarricade();
		}
		return result;
	}

	const float nearestTarget = FindTargets(ctx, enemyManager, soaps, molotovs);
	if (!IsHolding() && input.grabPressed) {
		float nearest = std::min(nearestTarget, grabRange_);
		for (const auto& barricade : barricades) {
			if (!barricade->CanGrab() || std::abs(barricade->GetCollision().min.y - ctx.position.y) > grabRange_) {
				continue;
			}
			const float distance = MathUtility::Length(barricade->GetGrabOffset(ctx.position));
			if (distance < nearest) {
				nearest = distance;
				heldBarricade_ = barricade.get();
			}
		}
		if (heldBarricade_) {
			heldBarricade_->Grab(ctx.position);
			result.grabbedBarricade = true;
			result.stopMovement = true;
			return result;
		}
	}

	UpdateItem(input, ctx, result);
	TryGrabEnemy(input, ctx, result);
	UpdateHeldEnemy(input, ctx);
	UpdateAttack(input, ctx, result);

	return result;
}

float PlayerGrab::FindTargets(const Context& ctx, EnemyManager* enemyManager, const std::list<std::unique_ptr<Soap>>& soaps, const std::list<std::unique_ptr<Molotov>>& molotovs) {
	nearEnemy_ = nullptr;
	nearItem_ = nullptr;

	// 何かを掴んでいる間は新しいターゲットを探さない
	if (IsHolding()) {
		return grabRange_;
	}

	// 一番近い敵(3D距離)
	float enemyMinDist = FLT_MAX;
	for (auto& enemy : enemyManager->GetEnemies()) {
		if (enemy->IsDead() || enemy->IsBurning()) {
			continue;
		}

		float dist = MathUtility::Length(enemy->GetPos() - ctx.position);
		if (dist < enemyMinDist && dist < grabRange_) {
			enemyMinDist = dist;
			nearEnemy_ = enemy.get();
		}
	}

	float itemMinDist = FLT_MAX;
	const auto findNearestItem = [&](const auto& items) {
		for (const auto& item : items) {
			if (!item->CanGrab()) {
				continue;
			}
			Vector3 diff = item->GetPos() - ctx.position;
			diff.y = 0.0f;
			const float distance = MathUtility::Length(diff);
			if (distance < itemMinDist && distance < grabRange_) {
				itemMinDist = distance;
				nearItem_ = item.get();
			}
		}
	};
	findNearestItem(soaps);
	findNearestItem(molotovs);

	// 両方ある場合は近い方を優先
	if (nearItem_ && nearEnemy_) {
		if (itemMinDist < enemyMinDist) {
			nearEnemy_ = nullptr;
		} else {
			nearItem_ = nullptr;
		}
	}
	return std::min(itemMinDist, ctx.enableAttack ? enemyMinDist : FLT_MAX);
}

void PlayerGrab::ValidateBarricade(const std::list<std::unique_ptr<Barricade>>& barricades) {
	// エディターで削除された場合は参照を破棄する
	if (heldBarricade_ && std::none_of(barricades.begin(), barricades.end(), [this](const auto& barricade) {
		    return barricade.get() == heldBarricade_ && (barricade->IsPulling() || barricade->IsShield());
	    })) {
		heldBarricade_ = nullptr;
	}
}

void PlayerGrab::ReleaseBarricade() {
	if (heldBarricade_) {
		heldBarricade_->Release();
		heldBarricade_ = nullptr;
	}
}

void PlayerGrab::PlaceShield(const Vector3& position, const Vector2& forward, float yaw) {
	if (IsHoldingShield()) {
		heldBarricade_->PlaceShield(position, forward, yaw);
	}
}

bool PlayerGrab::IsPullingBarricade() const {
	return heldBarricade_ && heldBarricade_->IsPulling();
}

bool PlayerGrab::IsHoldingShield() const {
	return heldBarricade_ && heldBarricade_->IsShield();
}

void PlayerGrab::UpdateItem(const Input& input, const Context& ctx, Result& result) {
	const Vector3 forward = {ctx.forward.x, 0.0f, ctx.forward.y};

	// 掴む
	if (!IsHolding() && input.grabPressed && nearItem_) {
		heldItem_ = nearItem_;
		heldItem_->Grab();
		result.grabbedItem = true;
		result.stopMovement = true;
	}

	if (heldItem_) {
		if (input.attackPressed) {
			// 掴んでいる投擲アイテムを投げる
			heldItem_->Throw(forward);
			heldItem_ = nullptr;
			result.stopMovement = true;
		} else if (input.grabHeld) {
			// 掴んでいる間はプレイヤーの前方に固定
			Vector3 holdPos = heldItem_->GetPos();
			holdPos.x = ctx.position.x + forward.x * holdOffset_;
			holdPos.z = ctx.position.z + forward.z * holdOffset_;
			heldItem_->SetPos(holdPos);
		} else {
			// 掴みボタンを離したらその場に置く
			heldItem_ = nullptr;
		}
	} else if (!IsHolding() && nearItem_ && input.attackPressed) {
		// 掴まずに直接突き飛ばす
		nearItem_->Throw(forward);
		result.stopMovement = true;
	}
}

void PlayerGrab::TryGrabEnemy(const Input& input, const Context& ctx, Result& result) {
	if (!ctx.enableAttack || IsHolding() || !input.grabPressed || !nearEnemy_) {
		return;
	}

	heldEnemy_ = nearEnemy_;
	heldEnemy_->SetEnableAI(false);
	heldEnemy_->SetShotHoldState(true);
	heldEnemy_->SetAIState(EnemyAI::State::Hold);
	heldEnemy_->ResetShotTimer();

	result.grabbedEnemy = true;
	result.stopMovement = true;
}

void PlayerGrab::UpdateHeldEnemy(const Input& input, const Context& ctx) {
	if (!heldEnemy_) {
		return;
	}

	// 掴んでいる敵が死亡していたら掴み状態を解除
	if (heldEnemy_->IsDead()) {
		heldEnemy_ = nullptr;
		return;
	}

	if (input.grabHeld) {
		// 掴んでいる間はプレイヤーの前方に敵を固定
		Vector3 holdPos = {ctx.position.x + ctx.forward.x * holdOffset_, ctx.position.y, ctx.position.z + ctx.forward.y * holdOffset_};
		heldEnemy_->SetPos(holdPos);
		heldEnemy_->SetRotate(ctx.rotate);
	} else if (input.grabReleased) {
		ReleaseEnemy();
	}
}

void PlayerGrab::UpdateAttack(const Input& input, const Context& ctx, Result& result) {
	if (!ctx.enableAttack || !input.attackPressed) {
		return;
	}

	// 投げる対象
	Enemy* throwTarget = nullptr;
	if (heldEnemy_) {
		throwTarget = heldEnemy_;
		heldEnemy_ = nullptr; // 投げたのでポインタをクリア
	} else if (!IsHolding() && nearEnemy_) {
		throwTarget = nearEnemy_;
	}

	if (throwTarget) {
		throwTarget->SetEnableAI(true);
		throwTarget->StartKnockBack({ctx.forward.x, 0.0f, ctx.forward.y});
		throwTarget->SetAIState(EnemyAI::State::Normal);
	}

	// 対象がいなくてもアクション自体は行う
	result.attacked = true;
	result.stopMovement = true;
}

void PlayerGrab::ReleaseEnemy() {
	if (!heldEnemy_) {
		return;
	}

	heldEnemy_->SetEnableAI(true);
	heldEnemy_->SetAIState(EnemyAI::State::Normal);
	heldEnemy_ = nullptr;
}

void PlayerGrab::ReleaseEnemyByGuard() {
	// 死亡している敵には触らずポインタだけ破棄
	if (heldEnemy_ && !heldEnemy_->IsDead()) {
		ReleaseEnemy();
	} else {
		heldEnemy_ = nullptr;
	}
}

bool PlayerGrab::GetPreviewParams(bool enableAttack, Vector3& outPos, float& outPower, float& outFriction) const {
	if (const auto* soap = dynamic_cast<const Soap*>(heldItem_)) {
		outPos = soap->GetPos();
		outPower = soap->GetThrowSpeed();
		outFriction = soap->GetFriction();
		return true;
	}

	if (heldEnemy_ && enableAttack) {
		outPos = heldEnemy_->GetPos();
		outPower = heldEnemy_->GetKnockBackPower();
		outFriction = heldEnemy_->GetKnockBackFriction();
		return true;
	}

	return false;
}
