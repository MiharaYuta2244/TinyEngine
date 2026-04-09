#include "Model.h"
#include "MathUtility.h"
#include "ModelCommon.h"
#include "SphereMeshGenerator.h"
#include "TextureManager.h"
#include "TransformationMatrix.h"
#include "DirectXUtils.h"
#include <Transform.h>
#include <assert.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fstream>
#include <sstream>

using namespace Microsoft::WRL;

void Model::Initialize(ModelCommon* modelCommon, TextureManager* textureManager, const std::string& filename) {
	modelCommon_ = modelCommon;
	textureManager_ = textureManager;
	filename_ = filename;

	// モデル読み込み
	modelData_ = LoadModelFile(filename_);

	// 頂点データの初期化
	CreateVertexData();

	// テクスチャ読み込み
	textureManager_->LoadTexture(modelData_.material.textureFilePath);

	// テクスチャ番号を取得して、メンバ変数に書き込む
	modelData_.material.textureIndex = textureManager_->GetSrvIndex(modelData_.material.textureFilePath);
}

void Model::Update() {}

void Model::Draw() {
	auto commandList = modelCommon_->GetDxCommon()->GetCommandList();

	// VertexBufferViewを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定
	// SRVのDescriptorTableの先頭を設定。3はrootParameter[3]（Pixel用テクスチャ）である。
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetSrvHandleGPU(modelData_.material.textureFilePath));
	// 描画!(DrawCall/ドローコール)。
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);
}

ModelData Model::LoadModelFile(const std::string& filename) {
	ModelData modelData;            // 構築するModelData
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals;   // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line;               // ファイルから読んだ1行を格納するもの

	// デフォルトのテクスチャパスを事前に設定
	modelData.material.textureFilePath = "resources/model/white.png";

	Assimp::Importer importer;
	std::string filePath = "resources/model/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes()); // メッシュがないのは対応しない

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());        // 法線がないMeshは今回は非対応
		assert(mesh->HasTextureCoords(0)); // TexcoordがないMeshは今回は非対応

		// Meshの中身(Face)の解析を行っていく
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3); // 三角形のみサポート

			// Faceの中身(Vertex)の解析を行っていく
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				VertexData vertex;
				vertex.position = {position.x, position.y, position.z, 1.0f};
				vertex.normal = {normal.x, normal.y, normal.z};
				vertex.texcoord = {texcoord.x, texcoord.y};

				// aiProcess_MaskLeftHandedはz*=-1で、右手->左手に変換するので手動で対処
				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;
				modelData.vertices.push_back(vertex);
			}
		}
	}

	// マテリアルの中身を解析していく
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);

			// テクスチャファイルパスが空でない場合は指定したパスを使用
			if (textureFilePath.length > 0) {
				modelData.material.textureFilePath = "resources/model/" + std::string(textureFilePath.C_Str());
			}
		}
	}

	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& filename) {
	MaterialData materialData;                         // 構築するMaterialData
	std::string line;                                  // ファイルから読んだ1行を格納するもの
	std::ifstream file("resources/model/" + filename); // ファイルを開く
	assert(file.is_open());                            // とりあえず開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = "resources/model/" + textureFilename;
		}
	}
	return materialData;
}

void Model::CreateVertexData() {
	// 頂点リソースの作成
	vertexResource_ = DirectXUtils::CreateBufferResource(modelCommon_->GetDxCommon()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	vertexResource_->Unmap(0, nullptr);
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}

void Model::CreateIndexData() {
	// 球のメッシュを生成
	SphereMeshGenerator sphereMesh(16);
	meshData_ = sphereMesh.GenerateMeshData();

	indexCount_ = static_cast<uint32_t>(meshData_.indices.size());

	// インデックスリソースの作成
	indexResource_ = DirectXUtils::CreateBufferResource(modelCommon_->GetDxCommon()->GetDevice(), sizeof(uint32_t) * indexCount_);
	// リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * indexCount_;
	// 1頂点あたりのサイズ
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	// 書き込むためのアドレスを取得
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	indexResource_->Unmap(0, nullptr);
	std::memcpy(indexData_, meshData_.vertices.data(), sizeof(uint32_t) * indexCount_);
}

Node Model::ReadNode(aiNode* node) {
	Node result;
	aiMatrix4x4 aiLocalMatrix = node->mTransformation; // nodeのlocalMatrixを取得
	aiLocalMatrix.Transpose();                         // 別ベクトル形式を行ベクトル形式に転置

	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result.localMatrix.m[row][col] = aiLocalMatrix[row][col];
		}
	}

	result.name = node->mName.C_Str();          // Node名を格納
	result.children.resize(node->mNumChildren); // 子供の数だけ確保
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		// 再帰的に読んで階層構造を作っていく
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}

	return result;
}