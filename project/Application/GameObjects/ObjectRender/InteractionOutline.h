#pragma once
#include "ObjectRender.h"
#include "JsonManager.h"
#include <algorithm>
#include <array>
#include <cmath>
#ifdef USE_IMGUI
#include "imgui.h"
#endif

// インタラクト対象だけが持つ設定。各ステージの既存JSONに保存する
struct InteractionOutline {
	bool enabled = true;
	Vector4 color{1.0f, 0.75f, 0.15f, 1.0f};
	float thickness = 0.06f; // ワールド単位

	void Apply(ObjectRender& render, bool interactable) const {
		render.SetEnableOutline(enabled && interactable && thickness > 0.0f && color.w > 0.0f);
		render.SetOutlineColor(color);
		render.SetOutlineThickness(thickness);
	}

	void Save(nlohmann::json& j) const {
		j["outline"] = {{"enabled", enabled}, {"color", {color.x, color.y, color.z, color.w}}, {"thickness", thickness}};
	}

	void Load(const nlohmann::json& j) {
		if (!j.contains("outline")) {
			return; // 既存データはデフォルトで表示
		}
		try {
			const auto& data = j.at("outline");
			InteractionOutline loaded;
			loaded.enabled = data.value("enabled", enabled);
			loaded.thickness = data.value("thickness", thickness);
			if (data.contains("color")) {
				const auto values = data.at("color").get<std::array<float, 4>>();
				loaded.color = {values[0], values[1], values[2], values[3]};
			}
			if (!std::isfinite(loaded.thickness) || !std::isfinite(loaded.color.x) || !std::isfinite(loaded.color.y) ||
			    !std::isfinite(loaded.color.z) || !std::isfinite(loaded.color.w)) {
				return;
			}
			loaded.thickness = std::clamp(loaded.thickness, 0.0f, 1.0f);
			loaded.color.x = std::clamp(loaded.color.x, 0.0f, 1.0f);
			loaded.color.y = std::clamp(loaded.color.y, 0.0f, 1.0f);
			loaded.color.z = std::clamp(loaded.color.z, 0.0f, 1.0f);
			loaded.color.w = std::clamp(loaded.color.w, 0.0f, 1.0f);
			*this = loaded;
		} catch (const nlohmann::json::exception& e) {
			Logger::Log(std::string("[InteractionOutline] Invalid settings: ") + e.what(), LogLevel::Warning);
		}
	}

	void DrawImGui() {
#ifdef USE_IMGUI
		if (ImGui::TreeNode("Interaction Outline")) {
			ImGui::Checkbox("Enabled", &enabled);
			ImGui::ColorEdit4("Outline Color", &color.x);
			ImGui::DragFloat("Thickness (world)", &thickness, 0.005f, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::TreePop();
		}
#endif
	}
};
