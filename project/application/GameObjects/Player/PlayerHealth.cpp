#include "PlayerHealth.h"
#include "imGuiManager/ImGuiManager.h"

void PlayerHealth::Initialize(float maxHP){
	// HPの設定
	maxHP_ = maxHP;
	hp_ = maxHP;
}

void PlayerHealth::Damage(float value) {
	// ダメージ処理
	hp_ -= value;
}

void PlayerHealth::AllHeal() {
	// HP全回復
	hp_ = maxHP_;
}

void PlayerHealth::DrawImGui() {
#ifdef USE_IMGUI
ImGui::Begin("PlayerHP");
ImGui::DragFloat("currentHP", &hp_, 1.0f);
ImGui::DragFloat("maxHP", &maxHP_, 1.0f);
ImGui::End();
#endif // USE_IMGUI
}

bool PlayerHealth::IsDead() const { 
	if(hp_ <= 0.0f){
		return true;
	}

	return false; 
}
