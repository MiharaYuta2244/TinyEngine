#pragma once
#include <memory>
#include "EngineContext.h"
#include "Object3d.h"

class TitleText {
public:
	void Initialize(EngineContext* ctx);
	void Update(float deltaTime);
	void Draw();

private:
	std::unique_ptr<Object3d> textModel_;

	float baseScale_ = 12.0f; // 基本の大きさ
	float amplitude_ = 10.0f; // 拡縮の幅
	float beatInterval_ = 10.0f;

	float timer_ = 0.0f;
	const float kTimer = 3.0f;
};
