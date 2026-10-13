#pragma once
#include "AABB.h"
#include "Transform.h"

/// <summary>
/// ゲームオブジェクトの基底クラス
/// </summary>
class IGameObject {
public:
	virtual ~IGameObject() = default;

	Transform& GetTransform() { return transform_; }

	// ImGuiのリスト表示用に使用する名前
	virtual std::string GetName() const { return "GameObject"; }

	// ギズモ用の当たり判定
	virtual AABB GetAABBForGizmo() const { return aabbForGizmo_; }

	// 固有パラメータのImGui編集UI
	virtual void DrawExtraImGui() {}

protected:
	// ギズモ用の当たり判定の更新
	void UpdateAABBForGizmo();

protected:
	Transform transform_{};
	AABB aabbForGizmo_;
};