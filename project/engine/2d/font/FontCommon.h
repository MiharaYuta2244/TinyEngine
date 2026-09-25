#pragma once
#include "DirectXCommon.h"
#include <dxcapi.h>
#include <wrl.h>

/// <summary>
/// フォント(TextSprite)描画の共通クラス
/// </summary>
class FontCommon {
public:
	// 初期化関数
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingCommon();

	// getter
	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

private:
	// ルートシグネチャの作成
	void CreateRootSignature();

	// グラフィックスパイプラインの生成
	void CreateGraphicsPipeline();

	// シェーダコンパイラの初期化
	void InitializeShaderCompiler();

private:
	DirectXCommon* dxCommon_ = nullptr;

	// Root Signature / Pipeline
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// Shader Compiler
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
};