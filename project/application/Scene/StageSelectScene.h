#pragma once
#include "AudioManager.h"
#include "BaseScene.h"
#include "DecalManager.h"
#include "EasingAnimation.h"
#include "Font.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "Sprite.h"
#include "TextSprite.h"

/// <summary>
/// ステージセレクトシーン
/// </summary>
class StageSelectScene : public BaseScene {
public:
	// 初期化関数
	void Initialize(const SceneContext& ctx) override;

	// 更新関数
	void Update() override;

	// 描画関数
	void Draw() override;

	// 終了時処理
	void Finalize() override;

private:
	// 入力処理
	void HandleInput();

	// 選択変更
	void ChangeSelection(int delta);

	// カメラの更新
	void UpdateCamera(float deltaTime);

	// モデルの更新
	void UpdateModels(float deltaTime);

	// 選択リングの更新
	void UpdateRing(float deltaTime);

	// スクショ・タイム・撃破数の更新
	void UpdateInfo(float deltaTime);

	// マーカー色の反映
	void ApplyMarkerColors();

	// ステージiの弧上の角度
	float GetStageAngle(int index) const;

	// ステージ間をつなぐ点線の生成
	void CreateConnectionLine();

	// 弧上の座標
	Vector3 GetArcPos(float angle, float radius, float y) const;

private:
	// ステージ数
	static constexpr int kStageCount = CommonData::kStageCount;

	int selected_ = 0;
	bool isDecided_ = false;
	bool stickInUse_ = false;

	// カメラ
	std::unique_ptr<Camera> camera_;
	float currentAngle_ = 0.0f;              // カメラが向いている弧上の角度
	EasingAnimation<float> cameraAngleAnim_; // 角度のイージング
	float cameraMoveDuration_ = 0.8f;        // カメラの移動にかかる時間

	// 弧・カメラのパラメータ
	float arcStepRad_ = 0.55f;    // ステージ間の角度
	float arcRadius_ = 28.0f;      // 弧の半径
	float cameraDistance_ = 16.0f; // ステージからカメラまでの距離
	float cameraHeight_ = 7.0f;
	float cameraPitch_ = 0.28f;
	float cameraLateral_ = 2.5f; // カメラを左にずらす量

	// モデル
	std::array<std::unique_ptr<ObjectRender>, kStageCount> models_; // ステージモデル
	float modelTime_ = 0.0f;                                        // モデルのアニメーション用タイマー
	float modelScale_ = 1.5f;                                       // モデルのスケール
	float modelHeight_ = 2.5f;                                      // モデルの高さ
	float rotateSpeed_ = 0.6f;                                      // モデルの回転速度
	float bobSpeed_ = 1.5f;                                         // 縦揺れの速度
	float bobAmplitude_ = 0.3f;                                     // 振幅の強さ

	// デカール(マーカー・点線・選択リング)
	std::unique_ptr<TinyEngine::DecalManager> decalManager_;
	std::array<TinyEngine::DecalManager::DecalData*, kStageCount> markers_{};
	std::vector<TinyEngine::DecalManager::DecalData*> connectionDots_;
	std::vector<TinyEngine::DecalManager::DecalData*> ringDots_;
	float ringRotation_ = 0.0f;
	float ringSpeed_ = 1.0f;
	float ringRadius_ = 1.6f;

	// 2D
	std::unique_ptr<TinyEngine::Sprite> background_;                           // 背景
	std::unique_ptr<TinyEngine::Sprite> screenshotFrame_;                      // スクショ背景
	std::array<std::unique_ptr<TinyEngine::Sprite>, kStageCount> screenshots_; // ステージのスクショ
	float screenshotAlpha_ = 1.0f;                                             // スクショ透明度
	EasingAnimation<float> screenshotAlphaAnim_;                               // 透明度アニメーション

	std::unique_ptr<TinyEngine::Font> font_;                // フォント
	std::unique_ptr<TinyEngine::TextSprite> clearTimeText_; // クリアテキスト
	std::unique_ptr<TinyEngine::TextSprite> killCountText_; // 敵を倒した数テキスト

	std::unique_ptr<TinyEngine::AudioManager> audioManager_; // オーディオマネージャー

	// ポストエフェクトパラメータ
	ScanlineParam scanlineParam_;                 // 走査線
	BarrelDistortionParam barrelDistortionParam_; // 歪み
	GlitchParam glitchParam_;                     // グリッチ
};