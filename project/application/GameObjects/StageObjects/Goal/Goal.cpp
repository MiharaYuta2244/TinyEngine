#include "Goal.h"
#include "EffectLibrary.h"

using namespace TinyEngine;

void Goal::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	transform_.scale = {1.0f, 1.0f, 5.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {542.0f, 0.0f, -17.0f};

	// エフェクトの生成
	particle_.clear();
	EffectLibrary::Get().Spawn(ctx_, "GoalLine", transform_.translate, particle_);

	// SetPosやImGuiで座標が動いても、発生済みの粒ごと追従させる
	for (auto& particle : particle_) {
		particle->SetFollowTarget(true);
	}
}

void Goal::Update(float deltaTime) {
	Vector3 pos = transform_.translate;

	// 当たり判定更新
	Vector3 scale = transform_.scale;
	Vector3 halfScale;
	halfScale.x = scale.x / 2.0f;
	halfScale.y = scale.y / 2.0f;
	halfScale.z = scale.z / 2.0f;
	col_.max = {pos.x + halfScale.x, pos.y + halfScale.y, pos.z + halfScale.z};
	col_.min = {pos.x - halfScale.x, pos.y - halfScale.y, pos.z - halfScale.z};

	// パーティクルの更新
	for (auto& particle : particle_) {
		particle->UpdateTranslate(transform_.translate);
		particle->Update();
	}

	// ギズモ用当たり判定更新
	UpdateAABBForGizmo();

	#ifdef USE_IMGUI
	ImGui::Begin("Goal");
	ImGui::DragFloat3("Pos", &transform_.translate.x, 0.1f);
	ImGui::End();
	#endif
}

void Goal::Draw() {
	// パーティクルの描画
	for (auto& particle : particle_) {
		particle->Draw();
	}
}