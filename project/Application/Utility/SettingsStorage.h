#pragma once
#include "PostEffectSetting.h"
#include <filesystem>
#include <string>

/// <summary>
/// 入力、音量、ポストエフェクトの設定を保存・復元する
/// </summary>
namespace SettingsStorage {
	// ユーザー別の保存先を取得する。取得できない場合は作業フォルダーを使用する
	std::filesystem::path Path();

	// 保存と変更検出で共通利用する、一定の順序の文字列を生成する
	std::string Serialize(const PostEffectSettings& pfx);

	// 一時ファイルへ書き込んでから置き換え、保存の成否を返す
	bool Save(const PostEffectSettings& pfx);

	// ファイル全体を検証してから反映する。不正な場合は現在の設定を維持する
	void Load(PostEffectSettings& pfx);
}
