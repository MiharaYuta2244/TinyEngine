#pragma once
#include <memory>
#include "Transform.h"
#include "Object3d.h"

class Actor {
public:
	void Initialize();

	void Update();

	void Draw();

protected:
	Transform transform_ = {0.0f,0.0f,0.0f};
	Vector3 size_ = {0.0f, 0.0f, 0.0f};
	Vector3 collisionSize = {0.0f, 0.0f, 0.0f};
	bool isActive_ = true;

	std::unique_ptr<Object3d> object3d_ = nullptr;
};