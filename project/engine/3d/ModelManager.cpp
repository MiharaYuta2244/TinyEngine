#include "ModelManager.h"
#include <filesystem>

namespace fs = std::filesystem;

void ModelManager::Initialize(DirectXCommon* dxCommon, TextureManager* textureManager) {
	modelCommon_ = std::make_unique<ModelCommon>();
	modelCommon_->Initialize(dxCommon);

	textureManager_ = textureManager;
}

Model* ModelManager::FindModel(const std::string& filePath) { 
	// 読み込み済みモデルの検索
	if (models.contains(filePath)) {
	// 読み込み済みモデルを戻り値としてreturn
		return models.at(filePath).get();
	}

	// ファイル名一致なし
	return nullptr;
}

void ModelManager::LoadModel(const std::string& filePath) {
	// 読み込み済みモデルを検索
	if (models.contains(filePath)) {
		// 読み込み済みなら早期return
		return;
	}

	// モデルの生成とファイル読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(modelCommon_.get(), textureManager_, filePath);

	// モデルをmapコンテナに格納する
	models.insert(std::make_pair(filePath, std::move(model)));
}

void ModelManager::AllModelLoad(){
	std::string directoryPath = "resources/models/";

	for (const auto& entry : fs::directory_iterator(directoryPath)) {
		if (entry.is_regular_file()) {
			// ファイルの拡張子をチェック
			auto ext = entry.path().extension().string();
			if (ext == ".obj" || ext == ".gltf" || ext == ".glb") {
				// ファイル名を取得して読み込む
				LoadModel(entry.path().filename().string());
			}
		}
	}
}
