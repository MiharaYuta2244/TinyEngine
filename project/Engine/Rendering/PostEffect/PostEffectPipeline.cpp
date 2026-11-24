#include "PostEffectPipeline.h"

using namespace TinyEngine;

void PostEffectPipeline::Inititlize(DirectXCommon* dxCommon, SrvManager* srvManager, TextureManager* textureManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	textureManager_ = textureManager;

	fallbackCopy_ = std::make_unique<TinyEngine::CopyImage>();
	fallbackCopy_->Initialize(dxCommon_, PostEffectType::FullScreen, textureManager_);
}

void PostEffectPipeline::SetEffects(const std::vector<PostEffectType>& types) {
	passes_.clear();

	for (size_t i = 0; i < types.size(); ++i) {
		PostEffectPass pass;
		pass.type = types[i];

		// CopyImage生成
		pass.copyImage = std::make_unique<TinyEngine::CopyImage>();
		pass.copyImage->Initialize(dxCommon_, types[i], textureManager_);

		if (i < types.size() - 1) {
			pass.renderTexture = std::make_unique<RenderTexture>();
			pass.renderTexture->Initialize(dxCommon_, srvManager_, WinApp::kClientWidth, WinApp::kClientHeight);
		}

		passes_.push_back(std::move(pass));
	}
}

void PostEffectPipeline::Excute(DirectXCommon* dxCommon, SrvManager* srvManager, uint32_t inputColorSrv, uint32_t inputDepthSrv, RenderTexture* outputTarget) {
	uint32_t currentColorSrv = inputColorSrv;
	uint32_t currentDepthSrv = inputDepthSrv;

	// 最終出力先の開始/終了
	auto beginFinal = [&]() {
		if (outputTarget) {
			outputTarget->BeginRender(dxCommon_);
		} else {
			auto cmd = dxCommon_->GetCommandList();
			D3D12_CPU_DESCRIPTOR_HANDLE backBufferRTV = dxCommon_->GetCurrentBackBufferRTV();
			cmd->OMSetRenderTargets(1, &backBufferRTV, false, nullptr);

			D3D12_VIEWPORT vp = dxCommon_->CreateViewport();
			D3D12_RECT sc = dxCommon_->CreateScissor();
			cmd->RSSetViewports(1, &vp);
			cmd->RSSetScissorRects(1, &sc);
		}
	};

	auto endFinal = [&]() {
		if (outputTarget) {
			outputTarget->EndRender(dxCommon_);
		}
	};

	// 有効なパスだけを集める
	std::vector<PostEffectPass*> active;
	for (auto& pass : passes_) {
		if (IsEffectEnabled(pass.type)) {
			active.push_back(&pass);
		}
	}

	// 全部OFFならそのままコピー
	if (active.empty()) {
		beginFinal();
		fallbackCopy_->Draw(dxCommon_, srvManager_, currentColorSrv, currentDepthSrv);
		endFinal();
		return;
	}

	for (size_t i = 0; i < active.size(); ++i) {
		PostEffectPass& pass = *active[i];
		bool isLast = (i == active.size() - 1);

		if (!isLast) {
			assert(pass.renderTexture);
			pass.renderTexture->BeginRender(dxCommon_);
		} else {
			beginFinal();
		}

		pass.copyImage->Draw(dxCommon_, srvManager_, currentColorSrv, currentDepthSrv);

		if (!isLast) {
			pass.renderTexture->EndRender(dxCommon_);
			currentColorSrv = pass.renderTexture->GetSRVIndexColor();
			currentDepthSrv = pass.renderTexture->GetSRVIndexDepth();
		} else {
			endFinal();
		}
	}
}

TinyEngine::CopyImage* PostEffectPipeline::GetPass(PostEffectType type) {
	for (auto& pass : passes_) {
		if (pass.type == type) {
			return pass.copyImage.get();
		}
	}
	return nullptr;
}

void PostEffectPipeline::SetEffectEnabled(PostEffectType type, bool enabled) {
	if (enabled) {
		disabledTypes_.erase(type);
	} else {
		disabledTypes_.insert(type);
	}
}