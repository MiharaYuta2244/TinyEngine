#pragma once
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "GameObjects/PlaceableObject.h"
#include "OBB.h"

class Player;

/// <summary>
/// プレイヤーの装飾が入った宝箱
/// </summary>
class TreasureChest : public PlaceableObject {
public:
	CollisionTag GetTag() const override { return CollisionTag::TreasureChest; }
	std::string GetName() const override { return "TreasureChest"; }
	void OnCollisionEnter(const CollisionEvent& event, const CollisionContext& context) override;

	// 初期化関数
	void Initialize(EngineContext* ctx, const Transform& transform);

	// 更新関数
	void Update();

	// 描画関数
	void Draw() override;

	const OBB& GetCollision() const { return collision_; }
	bool IsOpen() const { return isOpen_; }

private:
	void Open(Player& player);
	void UpdateCollision();

private:
	ObjectRender render_;
	OBB collision_{};
	bool isOpen_ = false;
};
