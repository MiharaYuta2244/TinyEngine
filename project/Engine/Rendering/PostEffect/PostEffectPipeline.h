#pragma once
#include "CopyImage.h"
#include "DirectXCommon.h"
#include "PostEffectPass.h"
#include "RenderTexture.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include <unordered_set>

class PostEffectPipeline {
public:
	void Inititlize(DirectXCommon* dxCommon, SrvManager* srvManager, TextureManager* textureManager);

	void SetEffects(const std::vector<PostEffectType>& types);

	void Excute(DirectXCommon* dxCommon, SrvManager* srvManager, uint32_t inputColorSrv, uint32_t inputDepthSrv, RenderTexture* outputTarget = nullptr);

	TinyEngine::CopyImage* GetPass(PostEffectType type);

	// エフェクトの種類ごとのON/OFF
	void SetEffectEnabled(PostEffectType tyoe, bool enabled);
	bool IsEffectEnabled(PostEffectType type) const { return !disabledTypes_.contains(type); }

private:
	std::vector<PostEffectPass> passes_;

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	TextureManager* textureManager_ = nullptr;

	// 無効なポストエフェクト
	std::unordered_set<PostEffectType> disabledTypes_;

	// 有効なパスが1つも無いときに、そのままコピーするためのパス
	std::unique_ptr<TinyEngine::CopyImage> fallbackCopy_;
};
