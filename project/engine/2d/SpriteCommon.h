#pragma once
#include "DirectXCommon.h"
#include "DirectXUtils.h"

/// <summary>
/// スプライトの共通クラス
/// </summary>
class SpriteCommon {
public:
	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

	// 初期化関数
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingCommon();

private:
	/// <summary>
	/// ルートシグネチャの作成
	/// </summary>
	void CreateRootSignature();

	/// <summary>
	/// グラフィックスパイプラインの生成
	/// </summary>
	void CreateGraphicsPipeline();

	void InitializeShaderCompiler();

private:
	DirectXCommon* dxCommon_;

	// Root Signature / Pipeline
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

	// Shader Compiler
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
};
