#pragma once
#include <memory>
#include "Transform.h"
#include "Object3d.h"
#include "AABB.h"
#include "Sphere.h"

class Actor {
public:
	AABB GetAABB() { return aabb_; }
	Sphere GetSphere() { return sphere_; }

protected:
	Transform transform_ = {0.0f,0.0f,0.0f};
	Vector3 size_ = {0.0f, 0.0f, 0.0f};
	Vector3 collisionSize = {0.0f, 0.0f, 0.0f};
	bool isActive_ = true;
	AABB aabb_;
	Sphere sphere_;

	std::unique_ptr<Object3d> object3d_ = nullptr;
};