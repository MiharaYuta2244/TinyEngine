#include "EnemyManager.h"
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif

void EnemyManager::Initialize(EngineContext* ctx) {
	ctx_ = ctx;

	// CSV読み込み
	csvPath_ = "resources/csv/Enemies.csv";
	LoadFromCSV(csvPath_);
}

void EnemyManager::Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager) {
	// 死亡した敵をリストから削除
	enemies_.remove_if([](const std::unique_ptr<Enemy>& enemy) { return enemy->IsDead(); });

	// 生きている敵をすべて更新
	for (auto& enemy : enemies_) {
		enemy->Update(deltaTime, player, enemyBulletManager, wallManager);
	}
}

void EnemyManager::PostUpdate() {
	for (auto& enemy : enemies_) {
		enemy->PostUpdate();
	}
}

void EnemyManager::Draw() {
	for (auto& enemy : enemies_) {
		enemy->Draw();
	}
}

void EnemyManager::DrawImGui() {
#ifdef USE_IMGUI
	ImGui::Begin("Enemy Manager");

	ImGui::Text("Enemy Count: %d", (int)enemies_.size());

	// 敵の追加ボタン
	if (ImGui::Button("Add Enemy")) {
		auto newEnemy = std::make_unique<Enemy>();
		newEnemy->Initialize(ctx_, {0.0f, 0.0f, 0.0f});
		// デフォルト出現位置などを設定可能
		newEnemy->SetPos({0.0f, 0.0f, 0.0f});
		enemies_.push_back(std::move(newEnemy));
	}

	// CSV保存ボタン
	if (ImGui::Button("Save CSV")) {
		SaveToCSV(csvPath_);
	}

	ImGui::Separator();

	int index = 0;
	for (auto it = enemies_.begin(); it != enemies_.end();) {
		auto& enemy = *it;

		ImGui::PushID(index);
		std::string header = "Enemy " + std::to_string(index);

		if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			// 座標の変更
			Vector3& pos = enemy->GetPos();
			ImGui::DragFloat3("Position", &pos.x, 0.1f);

			// 敵の削除ボタン
			if (ImGui::Button("Delete")) {
				it = enemies_.erase(it);
				ImGui::PopID();
				continue; // eraseした場合はイテレータが進むので次へ
			}
		}

		++it;
		++index;
		ImGui::PopID();
	}

	ImGui::End();
#endif
}

void EnemyManager::LoadFromCSV(const std::string& filepath) {
	std::ifstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Failed to open CSV: " << filepath << std::endl;
		return;
	}

	enemies_.clear();

	std::string line;
	while (std::getline(file, line)) {
		// コメント行 or 空行はスキップ
		if (line.empty() || line[0] == '#')
			continue;

		std::stringstream ss(line);
		std::string item;

		float posX, posZ;

		// posX
		std::getline(ss, item, ',');
		posX = std::stof(item);

		// posZ
		std::getline(ss, item, ',');
		posZ = std::stof(item);

		Vector3 pos = {posX, 0.0f, posZ};

		// 敵生成
		auto enemy = std::make_unique<Enemy>();
		enemy->Initialize(ctx_, pos);

		enemies_.push_back(std::move(enemy));
	}

	file.close();
}

void EnemyManager::SaveToCSV(const std::string& filepath) {
	std::ofstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Failed to save CSV: " << filepath << std::endl;
		return;
	}

	// コメント行
	file << "# posX, posZ\n";

	for (auto& enemy : enemies_) {
		Vector3 pos = enemy->GetPos();
		file << pos.x << "," << pos.z << "\n";
	}

	file.close();
	std::cout << "Saved Enemy CSV: " << filepath << std::endl;
}