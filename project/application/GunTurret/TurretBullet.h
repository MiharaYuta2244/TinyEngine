#pragma once
#include "EngineContext.h"
#include "Object3d.h"
#include <memory>

class TurretBullet {
public:
	TurretBullet(EngineContext* ctx, Vector3 pos);

	void Initialize(EngineContext* ctx, Vector3 pos);

	void Update(float deltaTime, Vector3 direction);

	void Draw();

	// 範囲外に出たか判定
	bool IsOutside();

private:
	// 移動
	void Move(float deltaTime, Vector3 direction);

private:
	// モデル
	std::unique_ptr<Object3d> bulletModel_;

	// 速度
	float velocity_ = 10.0f;
};
