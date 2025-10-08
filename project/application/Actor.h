#pragma once
#include "Transform.h"
#include "Model.h"
#include <memory>

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

	std::unique_ptr<Model> model_ = nullptr;
};