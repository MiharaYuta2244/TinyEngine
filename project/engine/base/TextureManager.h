#pragma once
#include "DirectXTex.h"
#include "MaterialData.h"
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "TextureData.h"

class DirectXCommon;
class SrvManager;

/// <summary>
/// テクスチャマネージャークラス
/// </summary>
class TextureManager {
public:
	~TextureManager();

	// 初期化処理
	void Initialize(DirectXCommon* directXCommon, SrvManager* srvManager);

	// テクスチャの読み込み
	void LoadTexture(const std::string& filePath);

	// プロジェクト内のテクスチャの読み込み
	void AllTextureLoad();

	// テクスチャ番号からGPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);

	// メタデータを取得
	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);

	// SRVインデックスの取得
	uint32_t GetSrvIndex(const std::string& filePath);

	SrvManager* GetSrvManager() { return srvManager_; }

public:
	// SRVインデックスの開始番号
	static uint32_t kSRVIndexTop;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);
	void UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages);

private:
	// テクスチャデータ
	std::unordered_map<std::string,TextureData> textureDatas_;
	DirectXCommon* directXCommon_;

	// srvManager
	SrvManager* srvManager_;
};
