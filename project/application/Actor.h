#pragma once
#include <memory>
#include "Transform.h"
#include "Object3d.h"
#include "AABB.h"
#include "Sphere.h"

class Actor {
public:
	// Getter
	Object3d* GetObject3d() { return object3d_.get(); }
	AABB GetAABB() { return aabb_; }
	Sphere GetSphere() { return sphere_; }
	Transform GetTransform() { return transform_; }
	Vector3 GetTranslate() { return transform_.translate; }
	Vector3 GetRotate() { return transform_.rotate; }
	Vector3 GetScale() { return transform_.scale; }

	// Setter
	void SetModel(const std::string model) { object3d_->SetModel(model); }
	void SetTranslate(Vector3 translate) { transform_.translate = translate; }
	void SetRotate(Vector3 rotate) { transform_.rotate = rotate; }
	void SetScale(Vector3 scale) { transform_.scale = scale; }

protected:
	Transform transform_ = {0.0f,0.0f,0.0f};
	Vector3 size_ = {0.0f, 0.0f, 0.0f};
	Vector3 collisionSize = {0.0f, 0.0f, 0.0f};
	bool isActive_ = true;
	AABB aabb_;
	Sphere sphere_;

	std::unique_ptr<Object3d> object3d_ = nullptr;
};