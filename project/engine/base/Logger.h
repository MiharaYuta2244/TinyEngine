#pragma once
#include <string>
#include <wrl.h>

/// <summary>
/// ログを出力するクラス
/// </summary>
class Logger {
public:
	/// <summary>
	/// 文字列を表示する関数
	/// </summary>
	/// <param name="message">文字列</param>
	static void Log(const std::string& message);
};
