#pragma once
#include "Actor.h"

class Object3dCommon;
class TextureManager;
class ModelManager;

class PowerUpItem : public Actor{
public:
	void Initialize(Object3dCommon* obj3dCommon, TextureManager* texMane, ModelManager* ModelMane);

	void Update(float deltaTime);

	void Draw();

	void SetModel(const std::string model) { object3d_->SetModel(model); }

private:
	// 当たり判定の位置更新
	void UpdateCollisionPos();
};
