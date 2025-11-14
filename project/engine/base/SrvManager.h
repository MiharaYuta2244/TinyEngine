#pragma once
#include "DirectXCommon.h"

class SrvManager {
public:
	void Initialize(DirectXCommon* dxCommon);

	uint32_t Allocate();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	// SRV生成(テクスチャ用)
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT mipLevels);

	// SRV生成(Structured Buffer用)
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);

	void PreDraw();

	void SetGraphicsRootDescriptorTable(UINT rootParameterIndex, uint32_t srvIndex);

	// テクスチャ枚数上限チェック
	bool CheckTextureMax();

private:
	DirectXCommon* dxCommon_ = nullptr;

	// 最大SRV数(最大テクスチャ枚数)
	static const uint32_t kMaxSRVCount;

	// SRV用のデスクリプタサイズ
	uint32_t descriptorSize_;

	// SRV用デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

	// 次に使用するSRVインデックス
	uint32_t useIndex = 1;
};
