#include "WallManager.h"
#include <fstream>
#include <iostream>
#include <sstream>

void WallManager::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	// CSV読み込み
	csvPath_ = "resources/csv/Walls.csv";
	LoadFromCSV(csvPath_);
}

void WallManager::Update() {
	for (auto& wall : walls_) {
		wall->Update();
	}
}

void WallManager::Draw() {
	for (auto& wall : walls_) {
		wall->Draw();
	}
}

void WallManager::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Wall Editor");

	ImGui::Text("Wall Count: %d", (int)walls_.size());

	int index = 0;
	for (auto it = walls_.begin(); it != walls_.end();) {
		auto& wall = *it;
		WallStatus& s = wall->GetWallStatus();

		// ID 衝突防止
		ImGui::PushID(index);

		// 折りたたみヘッダー
		std::string header = "Wall " + std::to_string(index);
		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

			ImGui::DragFloat("Width", &s.width, 0.1f);
			ImGui::DragFloat("Depth", &s.depth, 0.1f);
			ImGui::DragFloat("CenterX", &s.centerX, 0.1f);
			ImGui::DragFloat("CenterZ", &s.centerZ, 0.1f);

			wall->SetWallStatus(s);

			// 削除ボタン
			if (ImGui::Button("Delete")) {
				it = walls_.erase(it);
				ImGui::PopID();
				continue;
			}
		}

		ImGui::PopID();
		++it;
		++index;
	}

	ImGui::Separator();

	// 壁追加
	if (ImGui::Button("Add Wall")) {
		WallStatus s = {1.0f, 1.0f, 0.0f, 0.0f};
		auto wall = std::make_unique<Wall>();
		wall->Initialize(ctx_, s);
		walls_.push_back(std::move(wall));
	}

	// CSV保存
	if (ImGui::Button("Save CSV")) {
		WriteToCSV(csvPath_);
	}

	ImGui::End();
#endif
}


void WallManager::LoadFromCSV(const std::string& filepath) {
	std::ifstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Failed to open CSV: " << filepath << std::endl;
		return;
	}

	walls_.clear();

	std::string line;
	while (std::getline(file, line)) {
		// コメント行 or 空行はスキップ
		if (line.empty() || line[0] == '#')
			continue;

		std::stringstream ss(line);
		std::string item;

		float width, depth, centerX, centerZ;

		// width
		std::getline(ss, item, ',');
		width = std::stof(item);

		// depth
		std::getline(ss, item, ',');
		depth = std::stof(item);

		// centerX
		std::getline(ss, item, ',');
		centerX = std::stof(item);

		// centerZ
		std::getline(ss, item, ',');
		centerZ = std::stof(item);

		WallStatus wallStatus = {width, depth, centerX, centerZ};

		// Wall生成
		auto wall = std::make_unique<Wall>();
		wall->Initialize(ctx_, wallStatus);

		walls_.push_back(std::move(wall));
	}

	file.close();
}

void WallManager::WriteToCSV(const std::string& filepath) {
	std::ofstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Failed to write CSV: " << filepath << std::endl;
		return;
	}

	// ヘッダー
	file << "# width,depth,centerX,centerZ\n";

	for (auto& wall : walls_) {
		const WallStatus& s = wall->GetWallStatus();

		file << s.width << "," << s.depth << "," << s.centerX << "," << s.centerZ << "\n";
	}

	file.close();
}
