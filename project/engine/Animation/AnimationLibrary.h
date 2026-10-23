#pragma once
#include "KeyframeAnimation.h"
#include <string>
#include <unordered_map>

/// <summary>
/// アニメーションクリップのキャッシュ
/// 同じファイルは1回だけ読み込み、以降は参照を返す
/// </summary>
class AnimationLibrary {
public:
	/// <summary>
	/// クリップを取得する
	/// </summary>
	/// <param name="file">resources/models/からの相対パス</param>
	/// <returns>キャッシュ済みクリップ</returns>
	static const Animation& Get(const std::string& file);

	/// <summary>
	/// 事前読み込み
	/// </summary>
	static void Preload(const std::string& file) { (void)Get(file); }

	/// <summary>
	/// 全クリップを破棄
	/// </summary>
	static void Clear();

private:
	// 静的初期化順序の問題を避けるため関数内staticで持つ
	static std::unordered_map<std::string, Animation>& Cache();
};
