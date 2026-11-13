#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <dxcapi.h>
#include <string>

/// <summary>
/// DirectX便利クラス
/// </summary>
class DirectXUtils {
public:
	// DXCとインクルードハンドラーを初期化する
	static void InitializeShaderCompiler(
	    Microsoft::WRL::ComPtr<IDxcUtils>& utils,
	    Microsoft::WRL::ComPtr<IDxcCompiler3>& compiler,
	    Microsoft::WRL::ComPtr<IDxcIncludeHandler>& includeHandler);

	// 指定されたルートシグネチャをシリアライズして生成する。
	static Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateRootSignature(
	    ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& description);

	/// <summary>
	/// リソースバッファを作成するユーティリティ関数
	/// </summary>
	/// <param name="device">デバイス</param>
	/// <param name="sizeBytes">バッファサイズ</param>
	/// <returns>作成されたリソースバッファ</returns>
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeBytes);

	/// <summary>
	/// ShaderをCompilerするユーティリティ関数
	/// </summary>
	/// <param name="filePath">CompilerするShaderファイルへのパス</param>
	/// <param name="profile">Compilerに使用するProfile</param>
	/// <param name="dxcUtils">初期化で生成したIDxcUtilsインスタンス</param>
	/// <param name="dxcCompiler">初期化で生成したIDxcCompiler3インスタンス</param>
	/// <param name="includeHandler">初期化で生成したIDxcIncludeHandlerインスタンス</param>
	/// <returns>コンパイルされたシェーダーブロブ</returns>
	static IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);
};
