#pragma once
#include <string>

/// <summary>
/// 文字列変換ユーティリティクラス
/// </summary>
class StringUtility {
public:
	static std::wstring ConvertString(const std::string& str);
	static std::string ConvertString(const std::wstring& str);
};
