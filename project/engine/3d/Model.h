#pragma once
#include <wrl.h>
#include "Material.h"
#include "MaterialData.h"
#include "ModelData.h"
#include "ModelCommon.h"
#include "MeshData.h"
#include "IMeshGenerator.h"
#include "Node.h"
#include <d3d12.h>
#include <string>
#include <vector>
#include <assimp/scene.h>

class TextureManager;

/// <summary>
/// モデルクラス
/// </summary>
class Model {
public:
	// 初期化関数
	void Initialize(ModelCommon* modelCommon, TextureManager* textureManager, const std::string& filename);

	// 描画関数
	void Draw();

	// 更新関数
	void Update();

	// Getter
	const ModelData& GetModelData() const { return modelData_; }

private:
	ModelData LoadModelFile(const std::string& filename);

	MaterialData LoadMaterialTemplateFile(const std::string& filename);

	/// <summary>
	/// 頂点データ作成
	/// </summary>
	void CreateVertexData();

	/// <summary>
	/// インデックスデータ作成
	/// </summary>
	void CreateIndexData();

	// assimpのNode(aiNode)から、右の構造体に変換する関数を作る
	Node ReadNode(aiNode* node);

private: 
	ModelCommon* modelCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	std::string filename_;

	// objファイルのデータ
	ModelData modelData_;

	// メッシュデータ
	MeshData meshData_;

	// VertexBufferView
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;

	// Resource
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;

	// Resourceにデータを書き込むためのポインタ
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;

	// インデックスカウント
	uint32_t indexCount_;
};
