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

private:
	// 移動
	void Move(float deltaTime, Vector3 direction);

private:
	// モデル
	std::unique_ptr<Object3d> bulletModel_;
};
