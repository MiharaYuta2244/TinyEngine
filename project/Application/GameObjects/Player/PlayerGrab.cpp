#include "PlayerGrab.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "GameObjects/Item/Soap.h"
#include "GameObjects/StageObjects/Barricade/Barricade.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include <cfloat>
#include <algorithm>

PlayerGrab::Result PlayerGrab::Update(
    const Input& input, const Context& ctx, EnemyManager* enemyManager, const std::list<std::unique_ptr<Soap>>& soaps,
    const std::list<std::unique_ptr<Barricade>>& barricades) {
	Result result;

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

	const float nearestTarget = FindTargets(ctx, enemyManager, soaps);
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

	UpdateSoap(input, ctx, result);
	TryGrabEnemy(input, ctx, result);
	UpdateHeldEnemy(input, ctx);
	UpdateAttack(input, ctx, result);

	return result;
}

float PlayerGrab::FindTargets(const Context& ctx, EnemyManager* enemyManager, const std::list<std::unique_ptr<Soap>>& soaps) {
	nearEnemy_ = nullptr;
	nearSoap_ = nullptr;

	// 何かを掴んでいる間は新しいターゲットを探さない
	if (IsHolding()) {
		return grabRange_;
	}

	// 一番近い敵(3D距離)
	float enemyMinDist = FLT_MAX;
	for (auto& enemy : enemyManager->GetEnemies()) {
		if (enemy->IsDead()) {
			continue;
		}

		float dist = MathUtility::Length(enemy->GetPos() - ctx.position);
		if (dist < enemyMinDist && dist < grabRange_) {
			enemyMinDist = dist;
			nearEnemy_ = enemy.get();
		}
	}

	// 一番近い石鹸
	float soapMinDist = FLT_MAX;
	for (const auto& soap : soaps) {
		// 破壊済みの石鹸は掴めない
		if (soap->IsBroken()) {
			continue;
		}

		Vector3 diff = soap->GetPos() - ctx.position;
		diff.y = 0.0f;
		float dist = MathUtility::Length(diff);

		if (dist < soapMinDist && dist < grabRange_) {
			soapMinDist = dist;
			nearSoap_ = soap.get();
		}
	}

	// 両方ある場合は近い方を優先
	if (nearSoap_ && nearEnemy_) {
		if (soapMinDist < enemyMinDist) {
			nearEnemy_ = nullptr;
		} else {
			nearSoap_ = nullptr;
		}
	}
	return std::min(soapMinDist, ctx.enableAttack ? enemyMinDist : FLT_MAX);
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

void PlayerGrab::UpdateSoap(const Input& input, const Context& ctx, Result& result) {
	const Vector3 forward = {ctx.forward.x, 0.0f, ctx.forward.y};

	// 掴む
	if (!IsHolding() && input.grabPressed && nearSoap_) {
		heldSoap_ = nearSoap_;
		heldSoap_->Grab();
		result.grabbedSoap = true;
		result.stopMovement = true;
	}

	if (heldSoap_) {
		if (input.attackPressed) {
			// 掴んでいる石鹸を投げる
			heldSoap_->Throw(forward);
			heldSoap_ = nullptr;
			result.stopMovement = true;
		} else if (input.grabHeld) {
			// 掴んでいる間はプレイヤーの前方に固定
			Vector3 holdPos = heldSoap_->GetPos();
			holdPos.x = ctx.position.x + forward.x * holdOffset_;
			holdPos.z = ctx.position.z + forward.z * holdOffset_;
			heldSoap_->SetPos(holdPos);
		} else {
			// 掴みボタンを離したらその場に置く
			heldSoap_ = nullptr;
		}
	} else if (nearSoap_ && input.attackPressed) {
		// 掴まずに直接突き飛ばす
		nearSoap_->Throw(forward);
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
	if (heldSoap_) {
		outPos = heldSoap_->GetPos();
		outPower = heldSoap_->GetThrowSpeed();
		outFriction = heldSoap_->GetFriction();
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
