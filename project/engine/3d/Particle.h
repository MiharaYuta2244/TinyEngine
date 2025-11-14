#pragma once
#include "CameraForGPU.h"
#include "DebugCamera.h"
#include "DirectionalLight.h"
#include "FogParam.h"
#include "Material.h"
#include "MaterialData.h"
#include "ModelData.h"
#include "ModelManager.h"
#include "TimeParam.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include "VertexData.h"
#include <d3d12.h>
#include <string>
#include <vector>
#include <wrl.h>

class ParticleCommon;
class TextureManager;
class Model;

class Particle {
public:
	void Initialize(ParticleCommon* particleCommon, TextureManager* textureManager, ModelManager* modelManager);

	void Update();

	void Draw();

	// setter
	void SetModel(const std::string& filePath);
	void SetColor(Vector4 color);
	void SetWorldMatrix(Matrix4x4 worldMatrix) { worldMatrix_ = worldMatrix; }
	void SetCamera(DebugCamera* camera) { camera_ = camera; }

	// getter
	Vector4& GetColor() { return material_.color; }
	Matrix4x4& GetWorldMatrix() { return worldMatrix_; }
	Material& GetMaterial() { return material_; }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeBytes);

	// 四角形の作成
	ModelData CreatePrimitive();

	// 頂点データの初期化
	void CreateVertexData();

	// マテリアルの初期化
	void CreateMaterialData();

	// instancingResourceの作成
	void CreateInstancingResource();

private:
	ParticleCommon* particleCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	ModelManager* modelManager_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;

	// バッファリソース内のデータを指すポインタ
	uint32_t* indexData_ = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;

	// データ変更用の変数
	Material material_;
	Matrix4x4 worldMatrix_;

	// 描画する数
	static const uint32_t kNumInstance = 10;

	// 3Dオブジェクト自体とカメラの座標変換行列の元となるTransform
	Transform transforms_[kNumInstance];

	Matrix4x4 worldViewProjectionMatrix_;

	// カメラ
	DebugCamera* camera_ = nullptr;

	// モデルデータ
	ModelData modelData_;

	// VertexBufferView
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

	// Rsource
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	// Resourceにデータを書き込むためのポインタ
	VertexData* vertexData_ = nullptr;
	Material* materialData_ = nullptr;
	TransformationMatrix* instancingData_ = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU_;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_;
};
