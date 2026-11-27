#include "Glass.h"
#include "DebugCamera.h"
#include "GameObjects/Player/Player.h"
#include "ColorPalette.h"
#include "Random.h"

using namespace TinyEngine;
int Glass::index = 0;

Glass::Glass() { id_ = index++; }

void Glass::Initialize(EngineContext* ctx, const Transform& transform, TinyEngine::DecalManager* decalManager) {
	transform_ = transform;
	// 生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Cube.obj");
	render_->SetColor({1.0f, 1.0f, 1.0f, 0.2f});
	render_->SetEnableLighting(false);

	glassesDecalManager_ = decalManager;
}

void Glass::Update() {
	// 当たり判定の更新
	UpdateCollision();

	// 更新
	render_->Update(transform_);

	// ギズモ用当たり判定更新
	UpdateAABBForGizmo();
}

void Glass::Draw() {
	// 描画
	render_->Draw();
}

void Glass::UpdateCollision() {
	collision_ = AABB::FromCenterAndHalfExtentsXZ(transform_.translate, transform_.scale);
}

void Glass::AddGlassesDecal(Vector3 scale) {
	for (int i = 0; i < 50; ++i) {
		Vector3 basePos = transform_.translate;
		Vector3 pos = {
		    basePos.x + RandomUtils::RangeFloat(0.0f, 4.0f),
		    0,
		    basePos.z + RandomUtils::RangeFloat(-2.0f, 2.0f),
		};
		Vector3 rotate = {std::numbers::pi_v<float> / 2.0f, RandomUtils::RangeFloat(0.0f, std::numbers::pi_v<float>), 0.0f};
		glassesDecalManager_->AddDecal("Shard.png", pos, {rotate.x, rotate.y, rotate.z}, scale, ColorPalette::White());
	}
}

void Glass::OnCollisionEnter(const CollisionEvent& event, const CollisionContext& context) {
	if (!GetIsActive() || !event.other.CompareTag(CollisionTag::Player) || event.otherPart != CollisionPart::Attack) {
		return;
	}
	auto& player = static_cast<Player&>(event.other);
	AddGlassesDecal({0.1f, 0.1f, 0.1f});
	const Vector2 direction = player.GetDirection();
	context.onGlassBreak(*this, {direction.x, 0.0f, direction.y});
	if (!context.camera.GetIsShake()) {
		context.camera.StartShake(0.15f, 0.15f);
	}
	if (auto* rumble = player.GetRumbleManager()) {
		rumble->TriggerGlassBreak();
	}
	SetIsActive(false);
}
