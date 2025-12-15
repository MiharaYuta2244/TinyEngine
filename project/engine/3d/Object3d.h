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
#include "TimeParam.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include "VertexData.h"
#include "Model.h"
#include "EngineContext.h"
#include "PointLight.h"
#include "SpotLight.h"

class Object3d {
public:
	void Initialize(EngineContext* ctx);

	void Update();

	void Draw();

	// setter
	void SetModel(const std::string& filePath);
	void SetEnableFoging(const bool enableFoging) { material_.enableFoging = enableFoging; }
	void SetEnableLighting(const bool enableLighting) { material_.enableLighting = enableLighting; }
	void SetColor(Vector4 color);
	void SetModel(Model* model) { model_ = model; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
	void SetTransform(const Transform& transform) { transform_ = transform; }
	void SetWorldMatrix(Matrix4x4 worldMatrix) { worldMatrix_ = worldMatrix; }
	void SetCamera(DebugCamera* camera) { camera_ = camera; }
	void SetPointLightPosition(const Vector3& position) { pointLight_.position = position; }
	void SetPointLightColor(const Vector4& color) { pointLight_.color = color; }
	void SetPointLightIntensity(float intensity) { pointLight_.intensity = intensity; }
	void SetPointLightRadius(float radius) { pointLight_.radius = radius; }
	void SetPointLightDecay(float decay) { pointLight_.decay = decay; }
	void SetSpotLightPosition(const Vector3& position) { spotLight_.position = position; }
	void SetSpotLightColor(const Vector4& color) { spotLight_.color = color; }
	void SetSpotLightIntensity(float intensity) { spotLight_.intensity = intensity; }
	void SetSpotLightDirection(const Vector3& direction) { spotLight_.direction = direction; }
	void SetSpotLightDistance(float distance) { spotLight_.distance = distance; }
	void SetSpotLightDecay(float decay) { spotLight_.decay = decay; }
	void SetSpotLightCosAngle(float cosAngle) { spotLight_.cosAngle = cosAngle; }

	// getter
	Vector3& GetScale() { return transform_.scale; }
	Vector3& GetRotate() { return transform_.rotate; }
	Vector3& GetTranslate() { return transform_.translate; }
	Vector4& GetColor() { return material_.color; }

	Matrix4x4& GetWorldMatrix() { return worldMatrix_; }

	DirectionalLight& GetDirectionalLight() { return directionalLight_; }
	
	PointLight& GetPointLight() { return pointLight_; }
	
	SpotLight& GetSpotLight() { return spotLight_; }

	Material& GetMaterial() { return material_; }
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeBytes);

	/// <summary>
	/// 座標変換行列データ作成
	/// </summary>
	void CreateTransformationMatrixData();

	/// <summary>
	/// 平行光源データ作成
	/// </summary>
	void CreateDirectionalLightData();

	/// <summary>
	/// ポイントライトデータ作成
	/// </summary>
	void CreatePointLightData();

	/// <summary>
	/// スポットライトデータ作成
	/// </summary>
	void CreateSpotLightData();

	/// <summary>
	/// CameraForGPUデータの作成
	/// </summary>
	void CreateCameraForGPUData();

	/// <summary>
	/// フォグパラメータデータの作成
	/// </summary>
	void CreateFogParamData();

	/// <summary>
	/// タイムパラメータデータの作成
	/// </summary>
	void CreateTimeParamData();

	/// <summary>
	/// マテリアルデータの作成（インスタンス毎に持つ）
	/// </summary>
	void CreateMaterialData();

private:
	Model* model_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraForGPUResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> fogParamResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> timeParamResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	// バッファリソース内のデータを指すポインタ
	uint32_t* indexData_ = nullptr;
	TransformationMatrix* transformMatrixData_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;
	PointLight* pointLightData_ = nullptr;
	SpotLight* spotLightData_ = nullptr;
	CameraForGPU* cameraForGPUData_ = nullptr;
	FogParam* fogParamData_ = nullptr;
	TimeParam* timeParamData_ = nullptr;
	Material* materialData_ = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;

	// データ変更用の変数
	Material material_;
	Matrix4x4 worldMatrix_;
	DirectionalLight directionalLight_;
	PointLight pointLight_;
	SpotLight spotLight_;
	CameraForGPU cameraForGPU_;
	FogParam fogParam_;
	TimeParam timeParam_;

	// 3Dオブジェクト自体とカメラの座標変換行列の元となるTransform
	Transform transform_;

	Matrix4x4 worldViewProjectionMatrix_;

	// 描画する数
	static const uint32_t kNumInstance = 10;

	// カメラ
	DebugCamera* camera_ = nullptr;

	// コンテキスト構造体
	EngineContext* ctx_;
};
