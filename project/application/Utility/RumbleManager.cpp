#include "RumbleManager.h"

void RumbleManager::Initialize(GamePad* gamePad) { pad_ = gamePad; }

void RumbleManager::Update(float deltaTime) {
	rumbleTimer_.Update(deltaTime);

	// タイマーが終了したら振動を停止
	if (rumbleTimer_.IsEnd()) {
		if (pad_) {
			pad_->Rumble(0.0f, 0.0f, 0);
		}
	}
}

void RumbleManager::StartRumble(float left, float right, float duration) {
	if (pad_ && pad_->GetState().connected) {
		pad_->Rumble(left, right, 0);
		rumbleTimer_.Initialize(duration);
	}
}

void RumbleManager::StartRumble(const RumbleParam& param) { StartRumble(param.left, param.right, param.duration); }

void RumbleManager::TriggerPlayerDamage() { StartRumble(playerDamageParam_); }

void RumbleManager::TriggerEnemyDeath() { StartRumble(enemyDeathParam_); }

void RumbleManager::TriggerGlassBreak() { StartRumble(glassBreakParam_); }

void RumbleManager::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Rumble Settings");

	auto DrawParamGui = [](const char* label, RumbleParam& param) {
		if (ImGui::TreeNode(label)) {
			ImGui::DragFloat("Left Motor (Heavy)", &param.left, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Right Motor (Light)", &param.right, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Duration (sec)", &param.duration, 0.01f, 0.0f, 2.0f);
			ImGui::TreePop();
		}
	};

	DrawParamGui("Player Damage", playerDamageParam_);
	DrawParamGui("Enemy Death", enemyDeathParam_);
	DrawParamGui("Glass Break", glassBreakParam_);

	ImGui::End();
#endif
}