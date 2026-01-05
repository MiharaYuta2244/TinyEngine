#include "ResultModel.h"

void ResultModel::Initialize(EngineContext* ctx, std::string filename) {
	resultModel_ = std::make_unique<Object3d>();
	resultModel_->Initialize(ctx);
	resultModel_->SetModel(filename);
	resultModel_->SetTranslate({19.0f, 20.0f, 0.0f});
	resultModel_->SetScale({5.0f, 5.0f, 1.0f});
	resultModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
}

void ResultModel::Update(float deltaTime) {
	(void)deltaTime;
	resultModel_->Update();

#ifdef USE_IMGUI
	ImGui::Begin("resultModel");
	ImGui::DragFloat3("Translate", &resultModel_->GetTranslate().x, 0.1f);
	ImGui::DragFloat3("Scale", &resultModel_->GetScale().x, 0.1f);
	ImGui::End();
#endif
}

void ResultModel::Draw() { resultModel_->Draw(); }