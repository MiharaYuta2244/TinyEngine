#pragma once
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

/// <summary>
/// ログレベル
/// </summary>
enum class LogLevel { Info, Warning, Error, Debug };

/// <summary>
/// ログ管理クラス
/// </summary>
class Logger final {
public:
	// 初期化関数
	static void Initialize();

	// ログ出力
	static void Log(const std::string& message, LogLevel level);

private:
	// ログファイルの生成
	static void CreateLogFile();

	// ログレベルを文字列に変換
	static std::string LevelToString(LogLevel level);

private:
	// ログファイル
	static std::ofstream logStream_;
};
