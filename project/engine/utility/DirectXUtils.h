#pragma once
#include <d3d12.h>
#include <wrl.h>

class DirectXUtils {
public:
	/// <summary>
	/// リソースバッファを作成するユーティリティ関数
	/// </summary>
	/// <param name="device">デバイス</param>
	/// <param name="sizeBytes">バッファサイズ</param>
	/// <returns>作成されたリソースバッファ</returns>
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeBytes);
};
