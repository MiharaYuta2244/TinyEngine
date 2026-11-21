#include "TreasureChest.h"
#include "GameObjects/Player/Player.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include <cmath>

void TreasureChest::Initialize(EngineContext* ctx, const Transform& transform) {
	transform_ = transform;
	isOpen_ = false;
	render_.Initialize(ctx, "TreasureChest.obj");
	Update();
}

void TreasureChest::Update() {
	UpdateCollision();
	render_.Update(transform_);
}

void TreasureChest::Draw() {
	outline_.Apply(render_, !IsOpen());
	render_.Draw();
}

void TreasureChest::OnCollisionEnter(const CollisionEvent& event, const CollisionContext&) {
	if (!isOpen_ && event.otherPart == CollisionPart::Attack && event.other.CompareTag(CollisionTag::Player)) {
		auto& player = static_cast<Player&>(event.other);
		if (!player.IsDead()) {
			Open(player);
		}
	}
}

void TreasureChest::Open(Player& player) {
	isOpen_ = true;
	render_.SetModel("TreasureChestOpen.obj");
	render_.Update(transform_);

	// 開封時のHPで中身を決める。回復した段階に対応する装備はその場で身に着ける
	const float hp = player.GetCurrentHP();
	if (hp < 2.0f) {
		player.Heal(2.0f - hp); // 服
	} else if (hp < player.GetMaxHP()) {
		player.Heal(player.GetMaxHP() - hp); // 帽子
	} else {
		// HP満タン時の宝箱は空
	}
}

void TreasureChest::UpdateCollision() {
	const auto rotation = MathUtility::MakeAffineMatrix({1, 1, 1}, transform_.rotate, {});
	for (int axis = 0; axis < 3; ++axis) {
		collision_.orientations[axis] = {rotation.m[axis][0], rotation.m[axis][1], rotation.m[axis][2]};
	}

	collision_.center = transform_.translate - collision_.orientations[1] * (0.225f * transform_.scale.y);
	collision_.size = {
	    0.46f * std::abs(transform_.scale.x),
	    0.275f * std::abs(transform_.scale.y),
	    0.31f * std::abs(transform_.scale.z),
	};

	const Vector3 right = collision_.orientations[0] * collision_.size.x;
	const Vector3 up = collision_.orientations[1] * collision_.size.y;
	const Vector3 forward = collision_.orientations[2] * collision_.size.z;
	const Vector3 extent = {
	    std::abs(right.x) + std::abs(up.x) + std::abs(forward.x),
	    std::abs(right.y) + std::abs(up.y) + std::abs(forward.y),
	    std::abs(right.z) + std::abs(up.z) + std::abs(forward.z),
	};

	aabbForGizmo_ = {collision_.center - extent, collision_.center + extent};
}
