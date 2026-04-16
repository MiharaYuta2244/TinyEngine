#pragma once
#include "Model.h"
#include "TextureManager.h"
#include <map>
#include <memory>
#include <string>

/// <summary>
/// モデルマネージャークラス
/// </summary>
class ModelManager {
public:
	// 初期化関数
	void Initialize(DirectXCommon* dxCommon, TextureManager* textureManager);

	/// <summary>
	/// モデルの検索
	/// </summary>
	/// <param name="filePath">モデルのファイルパス</param>
	/// <returns>モデル</returns>
	Model* FindModel(const std::string& filePath);

	/// <summary>
	/// モデルファイルの読み込み
	/// </summary>
	/// <param name="filePath">ファイル名</param>
	void LoadModel(const std::string& filePath);

	/// <summary>
	/// プロジェクト内のすべてのモデルを読み込む
	/// </summary>
	void AllModelLoad();

private:
	std::unique_ptr<ModelCommon> modelCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	// モデルデータ
	std::map<std::string, std::unique_ptr<Model>> models;
};
