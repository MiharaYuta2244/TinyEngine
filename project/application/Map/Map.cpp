#include "Map.h"
#include <fstream>
#include <sstream>

bool Map::isMapLoaded_ = false;

void Map::Initialize() {
	// マップチップデータ読み込み格納
	if (!isMapLoaded_) {
		mapData1_ = LoadMapCSV("resources/map/map1.csv");
		SaveMapBinary(mapData1_, "resources/map/map1.bin"); // バイナリでマップデータの保存
		isMapLoaded_ = true;
	} else {
		// バイナリからマップデータを読み込み
		mapData1_ = LoadMapBinary("resources/map/map1.bin");
	}
}

std::vector<std::vector<int>> Map::LoadMapCSV(const std::string& filename) {
	std::vector<std::vector<int>> mapData;
	std::ifstream file(filename);
	std::string line;

	while (std::getline(file, line)) {
		std::vector<int> row;
		std::stringstream ss(line);
		std::string cell;

		while (std::getline(ss, cell, ',')) {
			row.push_back(std::stoi(cell));
		}

		mapData.push_back(row);
	}

	return mapData;
}

int Map::GetMapData(int row, int col) const {
	if (row < 0 || row >= GetRowCount() || col < 0 || col >= GetColumnCount()) {
		return static_cast<int>(TileType::EMPTY); // 範囲外は空タイル
	}
	return mapData1_[row][col];
}

void Map::SaveMapBinary(const std::vector<std::vector<int>>& mapData, const std::string& filename) {
	std::ofstream out(filename, std::ios::binary);
	if (!out)
		return;

	// 行数と列数を先頭に書いておくと便利
	int rows = static_cast<int>(mapData.size());
	int cols = static_cast<int>(mapData[0].size());
	out.write(reinterpret_cast<char*>(&rows), sizeof(int));
	out.write(reinterpret_cast<char*>(&cols), sizeof(int));

	// データ本体を書き込み
	for (const auto& row : mapData) {
		out.write(reinterpret_cast<const char*>(row.data()), row.size() * sizeof(int));
	}
}

std::vector<std::vector<int>> Map::LoadMapBinary(const std::string& filename) {
	std::ifstream in(filename, std::ios::binary);
	if (!in)
		return {};

	int rows, cols;
	in.read(reinterpret_cast<char*>(&rows), sizeof(int));
	in.read(reinterpret_cast<char*>(&cols), sizeof(int));

	std::vector<std::vector<int>> mapData(rows, std::vector<int>(cols));
	for (int r = 0; r < rows; ++r) {
		in.read(reinterpret_cast<char*>(mapData[r].data()), cols * sizeof(int));
	}

	return mapData;
}
