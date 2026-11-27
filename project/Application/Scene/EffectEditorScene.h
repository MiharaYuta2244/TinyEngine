#pragma once
#include "BaseScene.h"
#include "EffectData.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "Particle.h"
#include <memory>
#include <set>
#include <string>
#include <vector>

/// <summary>
/// エフェクトエディターシーン
/// </summary>
class EffectEditorScene : public BaseScene {
public:
	// 初期化関数
	void Initialize(const SceneContext& ctx) override;

	// 更新関数
	void Update() override;

	// 描画関数
	void Draw() override;

	// 終了処理
	void Finalize() override;

private:
#ifdef USE_IMGUI
	void DrawEffectList();
	void DrawEmitterList();
	void DrawInspector();
	void DrawPreviewControls();
	void HandleShortcuts();
	bool DrawEmitterSettings(EmitterData& e);
	bool DrawParams(ParticleParams& p);
	bool TextureCombo(EmitterData& e);
#endif
	void SelectEffect(const std::string& name);
	void MarkDirty();
	void UpdatePreview(float deltaTime);
	void RestartPreview();
	void RefreshTextureList();

private:
	std::string selectedEffect_;
	int selectedEmitter_ = -1;
	std::set<std::string> dirty_; // 未保存のエフェクト

	// プレビュー
	std::vector<std::unique_ptr<TinyEngine::Particle>> preview_;
	Vector3 previewPos_ = {0.0f, 0.5f, 0.0f};
	bool autoReplay_ = true;
	float replayInterval_ = 0.5f;
	float replayTimer_ = 0.0f;
	float restartDebounce_ = -1.0f;

	std::unique_ptr<ObjectRender> floor_;
	Transform floorTransform_{};

	std::vector<std::string> textureNames_;
	char nameBuf_[64] = "";
};