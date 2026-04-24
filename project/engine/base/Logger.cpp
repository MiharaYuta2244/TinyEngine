#include "Logger.h"
#include <format>
#include <windows.h>

std::ofstream Logger::logStream_;

void Logger::Initialize() {
	// ログファイルの生成
	CreateLogFile();

	// テスト出力
	Log("Logger Initialize", LogLevel::Info);
}

void Logger::Log(const std::string& message, LogLevel level) {
	std::string logLine = "[" + LevelToString(level) + "]" + message;

	// ファイル
	if (logStream_.is_open()) {
		logStream_ << logLine << std::endl;
	}

	// Visual Studio Output
	std::string debugOutput = logLine + "\n";
	OutputDebugStringA(debugOutput.c_str());
}

void Logger::Finalize() {
	logStream_.close();
}

void Logger::CreateLogFile() {
	// ログのディレクトリを用意
	std::filesystem::create_directories("logs");

	// 現在時刻を取得
	auto now = std::chrono::system_clock::now();

	// ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	auto nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);

	// 日本時間に変換
	std::chrono::zoned_time localTime{std::chrono::current_zone(), nowSeconds};

	// formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);

	// 時刻を使ってファイル名を決定
	std::string logFilePath = std::string("logs/") + dateString + ".log";

	// ファイルを作って書き込み準備
	logStream_.open(logFilePath);
}

std::string Logger::LevelToString(LogLevel level) {
	switch (level) {
	case LogLevel::Info:
		return "INFO";
	case LogLevel::Warning:
		return "WARNING";
	case LogLevel::Error:
		return "ERROR";
	case LogLevel::Debug:
		return "DEBUG";
	}
	return "UNKNOWN";
}
