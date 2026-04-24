#pragma once
#include "Logger.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

/// <summary>
/// Json管理クラス
/// </summary>
class JsonManager {
public:
	using json = nlohmann::json;

	// 任意の型をJsonファイルに保存
	template<typename T> static bool Save(const std::string& filepath, const T& data) {
		auto fullpath = "resources/json/" + filepath;

		try {
			json j = data;

			std::filesystem::path pathObj(fullpath);
			if (!std::filesystem::exists(pathObj.parent_path())) {
				// 親ディレクトリまでのパスを全て自動生成
				std::filesystem::create_directories(pathObj.parent_path());
			}

			std::ofstream file(fullpath);
			if (!file.is_open()) {
				std::cerr << "[JsonManager] Failed to open file for writing: " << fullpath << std::endl;
				Logger::Log("[JsonManager] Failed to open file for writing: " + fullpath, LogLevel::Error);
				return false;
			}

			file << j.dump(4);

			std::cerr << "[JsonManager] Save Success: " << fullpath << std::endl;
			Logger::Log("[JsonManager] Save Success: " + fullpath, LogLevel::Info);

			return true;
		} catch (const std::exception& e) {
			std::cerr << "[JsonManager] Save Error: " << e.what() << std::endl;
			Logger::Log("[JsonManager] Save Error: " + fullpath, LogLevel::Error);
			return false;
		}
	}

	// Jsonファイルから任意の型にデータを読み込む
	template<typename T> static bool Load(const std::string& filepath, T& outData) {
		auto fullpath = "resources/json/" + filepath;

		try {

			std::ifstream file(fullpath);
			if (!file.is_open()) {
				std::cerr << "[JsonManager] Failed to open file reading: " << fullpath << std::endl;
				Logger::Log("[JsonManager] Failed to open file reading: " + fullpath, LogLevel::Error);
				return false;
			}

			json j;
			file >> j;

			outData = j.get<T>();
			std::cerr << "[JsonManager] Load Success: " << fullpath << std::endl;
			Logger::Log("[JsonManager] Load Success: " + fullpath, LogLevel::Info);
			return true;
		} catch (const std::exception& e) {
			std::cerr << "[JsonManager] Load Error: " << e.what() << std::endl;
			Logger::Log("[JsonManager] Load Error: " + fullpath, LogLevel::Error);
			return false;
		}
	}
};
