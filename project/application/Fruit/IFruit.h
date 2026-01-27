#pragma once
#include "EngineContext.h"
#include "Object3d.h"
#include "AnimationBundle.h"
#include "AABB.h"
#include <memory>
#include <string>

class IFruit {
public:
	virtual void Initialize(EngineContext* ctx, const std::string& fruitName);
	virtual void Update(float deltaTime);
	virtual void Draw();

	// Getter
	std::string GetFruitName() { return fruitName_; }
	Vector3 GetTranslate() { return transform_.translate; }
	Vector3 GetRotate() { return transform_.rotate; }
	Vector3 GetScale() { return transform_.scale; }
	Transform GetTransform() { return transform_; }
	AABB GetAABB(){return aabb_;}

	// Setter
	void SetTranslate(Vector3 translate) { transform_.translate = translate; }
	void SetRotate(Vector3 rotate) { transform_.rotate = rotate; }
	void SetScale(Vector3 scale) { transform_.scale = scale; }
	void SetTransform(Transform transform) { transform_ = transform; }

	// フルーツの色を変更
	void SetFruitColor(Vector4 color) {
		if (object3d_) {
			object3d_->SetColor(color);
		}
	}

private:
	// 回転アニメーション
	void RotateAnimation(float deltaTime);

	// 回転アニメーション初期設定
	void SettingRotateAnimation();

protected:
	std::unique_ptr<Object3d> object3d_ = nullptr; // モデル

	std::string fruitName_{}; // 種類を判別するための名前

	Transform transform_{};

	AABB aabb_{};

	// 回転アニメーション
	AnimationBundle<float> rotateAnimation_;
};