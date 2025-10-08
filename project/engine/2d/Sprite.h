#pragma once

#include <wrl.h>
#include "Material.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include "VertexData.h"
#include <d3d12.h>
#include <memory>
#include <string>

class SpriteCommon;
class TextureManager;

class Sprite {
public:
	~Sprite();
	void Initialize(SpriteCommon* spriteCommon, TextureManager* textureManager, std::string textureFilePath);

	void Update();

	void Draw();

	/// <summary>
	/// 頂点データ作成
	/// </summary>
	void CreateVertexData();

	/// <summary>
	/// マテリアルデータ作成
	/// </summary>
	void CreateMaterialData();

	/// <summary>
	/// 座標変換行列データ作成
	/// </summary>
	void CreateTransformationData();

	// getter
	const Vector2& GetPosition() const { return position_; }
	float GetRotation() const { return rotation_; }
	const Vector4& GetColor() const { return materialData_->color; }
	const Vector2& GetSize() const { return size_; }
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() { return vertexBufferView_; }
	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetTransformMatrixResource() { return transformMatrixResource_; }

	// setter
	void SetPosition(const Vector2& position) { position_ = position; }
	void SetRotation(float rotation) { rotation_ = rotation; }
	void SetColor(const Vector4& color) { materialData_->color = color; }
	void SetSize(const Vector2& size) { size_ = size; }
	void SetSrvHandle(D3D12_GPU_DESCRIPTOR_HANDLE srvHandle);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeBytes);

private:
	SpriteCommon* spriteCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformMatrixResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;
	Material* materialData_ = nullptr;

	// バッファリソース内のデータを指すポインタ
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;

	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle_;

	Transform transform_;
	Transform uvTransform_;
	Matrix4x4 worldMatrix_;
	Material material_;

	// Spriteの座標,回転,サイズ
	Vector2 position_ = {0.0f, 0.0f};
	float rotation_ = 0.0f;
	Vector2 size_ = {640.0f, 360.0f};

	// テクスチャ番号
	uint32_t textureIndex_ = 0;
};
