#pragma once
#include "EngineContext.h"
#include "Object3d.h"
#include <memory>

class Cloud {
public:
	void Initialize(EngineContext* ctx);
	void Update(float deltaTime);
	void Draw();

private:
	std::unique_ptr<Object3d> cloudModel_;

	// 初期位置
	Vector3 startPos_ = {0.0f, 0.0f, 10.0f};

	// 移動速度
	float speed_ = 0.0f;
};
