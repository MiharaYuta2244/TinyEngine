#pragma once
#include <d3d12.h>
#include <string>
#include <vector>
#include <wrl.h>
#include "CameraForGPU.h"
#include "DebugCamera.h"
#include "DirectionalLight.h"
#include "FogParam.h"
#include "Material.h"
#include "MaterialData.h"
#include "ModelData.h"
#include "ModelManager.h"
#include "ParticleForGPU.h"
#include "ParticleState.h"
#include "TimeParam.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include "VertexData.h"
#include "AABB.h"
#include "DeltaTime.h"
#include "EngineContext.h"

class Particle {
public:
	void Initialize(EngineContext* ctx, Vector3 emitterPos);

	void Update();

	void Draw();

	// setter
	void SetModel(const std::string& filePath);
	void SetColor(Vector4 color);
	void SetWorldMatrix(Matrix4x4 worldMatrix) { worldMatrix_ = worldMatrix; }
	void SetCamera(DebugCamera* camera) { camera_ = camera; }
	void SetIsBillboard(bool isBillboard) { isBillboard_ = isBillboard; }

	// getter
	Vector4& GetColor() { return material_.color; }
	Matrix4x4& GetWorldMatrix() { return worldMatrix_; }
	Material& GetMaterial() { return material_; }

private:
	struct Emitter {
		Transform transform; // エミッタのトランスフォーム
		uint32_t count;      // 発生数
		float frequency;     // 発生頻度
		float frequencyTime; // 頻度用時刻
	};

	struct AccelerationField {
		Vector3 acceleration; // 加速度
		AABB area;            // 範囲
	};

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

	// パーティクル生成関数
	ParticleState MakeParticle(Vector3 translate);

	// BillboardMatrixを作成する
	Matrix4x4 CreateBillboardMatrix();

	// エミッターを使ってパーティクルの生成
	std::list<ParticleState> Emit(const Emitter& emitter, Vector3 translate);

	// 座標変換
	void CoordinateTransformation(std::list<ParticleState>::iterator particleIterator);

	// SRV作成
	void CreateInstancingSRV(UINT srvIndex);
	
	// エミッタの初期化
	void InitializeEmitter(Vector3 emitterPos);

	// 加速度フィールドの初期化
	void InitializeAccelerationField();

	// エミッタからパーティクルを生成する処理
	void UpdateEmitter();

	// パーティクル1つ更新
	void UpdateParticle(std::list<ParticleState>::iterator& itr);

	// GPU転送データへ書き込み
	void WriteParticleToGPU(const ParticleState& p, uint32_t index);

private:
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
	static const uint32_t kNumMaxInstance = 100;
	uint32_t numInstance_ = 0;

	// 3Dオブジェクト自体とカメラの座標変換行列の元となるTransform
	std::list<ParticleState> particles_;

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
	ParticleForGPU* instancingData_ = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU_;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_;

	// Δt
	const float kDeltaTime = 1.0f / 60.0f;

	// ビルボードするかどうか
	bool isBillboard_ = true;

	// エミッタ
	Emitter emitter{};

	// パーティクルに加速度を与える範囲
	AccelerationField accelerationField_;

	// 経過時間
	std::unique_ptr<DeltaTime> deltaTime_ = std::make_unique<DeltaTime>();

	// コンテキスト構造体
	EngineContext* ctx_;
};
