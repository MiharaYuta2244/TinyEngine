#pragma once
#include "DebugCamera.h"
#include "DirectXCommon.h"

/// <summary>
/// パーティクルの共通クラス
/// </summary>
class ParticleCommon {
public:
	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingCommon();

	void Initialize(DirectXCommon* dxCommon);

	// setter
	void SetDefaultCamera(DebugCamera* camera) { defaultCamera_ = camera; }

	// getter
	DirectXCommon* GetDxCommon() const { return dxCommon_; }
	DebugCamera* GetDefaultCamera() { return defaultCamera_; }

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

	// カメラ
	DebugCamera* defaultCamera_ = nullptr;
};
