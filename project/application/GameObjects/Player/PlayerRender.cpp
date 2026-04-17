#include "PlayerRender.h"

using namespace TinyEngine;

void PlayerRender::Initialize(EngineContext* ctx) {
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(ctx);
	object3d_->SetModel("suzanne.obj");
	object3d_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
}

void PlayerRender::Update(Transform transform) {
	object3d_->SetTransform(transform);
	object3d_->Update();
}

void PlayerRender::Draw() { object3d_->Draw(); }
