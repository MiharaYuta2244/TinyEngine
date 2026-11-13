#include "DirectXUtils.h"
#include "Logger.h"
#include "StringUtility.h"
#include <cassert>
#include <format>

using namespace Microsoft::WRL;

void DirectXUtils::InitializeShaderCompiler(
    ComPtr<IDxcUtils>& utils,
    ComPtr<IDxcCompiler3>& compiler,
    ComPtr<IDxcIncludeHandler>& includeHandler) {
	ComPtr<IDxcUtils> newUtils;
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&newUtils));
	assert(SUCCEEDED(hr));
	if (FAILED(hr)) {
		return;
	}

	ComPtr<IDxcCompiler3> newCompiler;
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&newCompiler));
	assert(SUCCEEDED(hr));
	if (FAILED(hr)) {
		return;
	}

	ComPtr<IDxcIncludeHandler> newIncludeHandler;
	hr = newUtils->CreateDefaultIncludeHandler(&newIncludeHandler);
	assert(SUCCEEDED(hr));
	if (FAILED(hr)) {
		return;
	}

	utils = newUtils;
	compiler = newCompiler;
	includeHandler = newIncludeHandler;
}

ComPtr<ID3D12RootSignature> DirectXUtils::CreateRootSignature(
    ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& description) {
	ComPtr<ID3DBlob> signatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(
	    &description, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			Logger::Log(static_cast<const char*>(errorBlob->GetBufferPointer()), LogLevel::Error);
		}
		assert(false);
		return {};
	}

	ComPtr<ID3D12RootSignature> rootSignature;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
	    signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
	return rootSignature;
}

ComPtr<ID3D12Resource> DirectXUtils::CreateBufferResource(ComPtr<ID3D12Device> device, size_t sizeBytes) {
	ComPtr<ID3D12Resource> resource;
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));

	return resource;
}

IDxcBlob* DirectXUtils::CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {
	// これからシェーダーをコンパイルする旨をログに出す
	Logger::Log(StringUtility::ConvertString(std::format(L"Begin CompileShader, path:{},profile:{}\n", filePath, profile)), LogLevel::Info);

	// hlslファイルを読む
	ComPtr<IDxcBlobEncoding> shaderSource;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	// 読めなかったら止める
	assert(SUCCEEDED(hr));

	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; // UTF8の文字コードであることを通知

	LPCWSTR arguments[] = {
	    filePath.c_str(), // コンパイル対象のhlslファイル名
	    L"-E",
	    L"main", // エントリーポイントの指定。基本的にmain以外にはしない
	    L"-T",
	    profile, // ShaderProfileの設定
	    L"-Zi",
	    L"-Qembed_debug", // デバッグ用の情報を埋め込む
	    L"-Od",           // 最適化を外しておく
	    L"-Zpr",          // メモリレイアウトは行優先
	};

	// 実際にShaderをコンパイルする
	ComPtr<IDxcResult> shaderResult;
	hr = dxcCompiler->Compile(&shaderSourceBuffer, arguments, _countof(arguments), includeHandler, IID_PPV_ARGS(&shaderResult));

	// コンパイルエラーではなくdxcが起動できないなど致命的な状況
	assert(SUCCEEDED(hr));

	// 警告・エラーが出てたらログにだして止める
	ComPtr<IDxcBlobUtf8> shaderError;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Logger::Log(shaderError->GetStringPointer(), LogLevel::Error);
		// 警告・エラーダメゼッタイ
		assert(false);
	}

	// コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	// 成功したログを出す
	Logger::Log(StringUtility::ConvertString(std::format(L"Compile Succeeded,path:{}, profile:{}\n", filePath, profile)), LogLevel::Info);

	// 実行用のバイナリを返却
	return shaderBlob;
}