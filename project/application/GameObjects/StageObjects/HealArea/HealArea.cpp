#include "HealArea.h"
#include "GameObjects/Player/Player.h"

using namespace TinyEngine;

int HealArea::index = 0;

HealArea::HealArea() { id_ = index++; }

void HealArea::Initialize(EngineContext* ctx, const Transform& transform) {
	transform_ = transform;
	transform_.scale.y = 1.0f;

	// 生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cube.obj");
	render_->SetColor(color_);
}

void HealArea::Update(float deltaTime) {
	// 当たり判定更新
	collision_ = AABB::FromCenterAndHalfExtentsXZ(transform_.translate, transform_.scale);

	// 更新
	render_->Update(transform_);
}

void HealArea::Draw() {
	if (!isActive_)
		return;

	// 描画
	render_->Draw();
}

void HealArea::OnCollisionEnter(const CollisionEvent& event, const CollisionContext&) {
	if (GetIsActive() && event.other.CompareTag(CollisionTag::Player) && event.otherPart == CollisionPart::Body) {
		static_cast<Player&>(event.other).AllHeal();
		SetIsActive(false);
	}
}
