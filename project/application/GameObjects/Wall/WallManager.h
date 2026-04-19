#pragma once
#include "Wall.h"
#include <list>

/// <summary>
/// 壁オブジェクト管理クラス
/// </summary>
class WallManager {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

	// ImGui
	void DrawImGui();

	// 壁のリストを取得するGetter
	const std::list<std::unique_ptr<Wall>>& GetWalls() const { return walls_; }

private:
	// CSV読み込み
	void LoadFromCSV(const std::string& filepath);

	// CSV書き込み
	void WriteToCSV(const std::string& filepath);

private:
	EngineContext* ctx_ = nullptr;
	std::string csvPath_;

	// 壁オブジェクトのリスト
	std::list<std::unique_ptr<Wall>> walls_;
};
