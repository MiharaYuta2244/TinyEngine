#pragma once
#include "Actor.h"
#include "EngineContext.h"

class PowerUpItem : public Actor{
public:
	void Initialize(EngineContext* ctx);

	void Update(float deltaTime);

	void Draw();

	void SetModel(const std::string model) { object3d_->SetModel(model); }

private:
	// 当たり判定の位置更新
	void UpdateCollisionPos();

	// コンテキスト構造体
	EngineContext* ctx_;
};
