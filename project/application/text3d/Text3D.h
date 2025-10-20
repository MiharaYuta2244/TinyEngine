#pragma once
#include "Actor.h"

class Object3dCommon;
class TextureManager;
class ModelManager;

class Text3D : public Actor {
public:
	void Initialize(Object3dCommon* obj3dCommon, TextureManager* texMane, ModelManager* ModelMane);

	void Update();

	void Draw();

	void SetModel(const std::string model) { object3d_->SetModel(model); }

	// Getter
	Object3d* GetObject3d() { return object3d_.get(); }

private:
	// 速度関連
	Vector3 velocity_ = {0.0f, 0.0f, 0.0f};
};
