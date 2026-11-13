#pragma once
#include "GameObjects/PlaceableObject.h"
#include "Matrix4x4.h"
#include <string>

class Decoration : public PlaceableObject {
public:
	void Initialize(EngineContext* ctx, const Transform& transform);

	CollisionTag GetTag() const override;
	std::string GetName() const override;
	Model* GetModel() const;

	Matrix4x4 GetModelWorldMatrix() const;

	AABB GetCollision() const;

	AABB GetAABBForGizmo() const override;

	// Stageがモデルごとにまとめて描画する
	void Draw() override;

	void ToExtraJson(Json& j) const override;

	void FromExtraJson(const Json& j) override;

	void DrawExtraImGui() override;

private:
	bool SetModel(const std::string& name);

	ModelManager* modelManager_ = nullptr;
	Model* model_ = nullptr;
	std::string modelName_;
};
