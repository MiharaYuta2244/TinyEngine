#pragma once
#include "DirectXTex.h"
#include <d3d12.h>
#include <string>
#include <wrl.h>
struct TextureData {
	std::string filepath;
	DirectX::TexMetadata metaData;
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
};