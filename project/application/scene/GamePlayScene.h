#pragma once
#include "BaseScene.h"
#include "Player.h"
#include "Enemy.h"
#include "Block.h"
#include "Map.h"
#include "PowerUpItem.h"
#include "Particle.h"
#include "Object3d.h"
#include "Sprite.h"
#include "BothCurtain.h"
#include <memory>
#include <vector>
#include <array>

class GamePlayScene : public BaseScene {
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// ゲームシーン初期化
	void StartGameScene();

	// マップチップに応じてオブジェクトの配置
	void SpawnObjectsByMapChip(Vector2 leftTop);

	// 当たり判定(Player, Enemy)
	void CollisionPlayerEnemy();

	// 当たり判定(Enemy, Player, ヒップドロップ)
	void CollisionEnemyPlayerHipDrop();

	// プレイヤーとパワーアップアイテムの当たり判定
	void CollisionPlayerPowerUpItem();

	// パワーアップアイテムの生成
	void CreatePowerUpItem();

	// シェイク初期設定用
	void StartShake(int frames, float magnitude);

	// シェイク関数
	void ShakeCamera();

#ifdef USE_IMGUI
	// フレームレートの表示
	void ImGuiFPS();

	// デバッグカメラ
	void ImGuiDebugCamera();
#endif

private:
	// プレイヤー
	std::unique_ptr<Player> player_;

	// マップ
	std::unique_ptr<Map> map_;

	// ブロック
	std::vector<std::unique_ptr<Block>> blocks_;

	// マップの左上座標
	Vector2 mapLeftTop_ = {0.0f, 0.0f};

	// 敵
	std::unique_ptr<Enemy> enemy_;

	// プレイヤーのパワーアップアイテム
	std::vector<std::unique_ptr<PowerUpItem>> powerUpItems_;

	// パワーアップアイテム生成フレームカウント
	int powerUpItemCreateFrameCount_ = 0;

	// パワーアップアイテムが生成されるまでの最大フレームカウント
	const int kPowerUpItemFrameCountMax = 60;

	// パワーアップアイテムの画面上に存在できる最大数
	const int kPowerUpItemCountMax = 5;

	// 土埃パーティクル プレイヤー用
	std::unique_ptr<Particle> particleDustPlayer_;

	// 土埃パーティクル 敵用
	std::unique_ptr<Particle> particleDustEnemy_;

	// 木のモデル
	std::array<std::unique_ptr<Object3d>, 10> treeModels_;

	// 地形モデル
	std::unique_ptr<Object3d> terrainModel_;

	// 画面両端の幕
	std::unique_ptr<BothCurtain> rightCurtain_;
	std::unique_ptr<BothCurtain> leftCurtain_;

	// ヒップドロップゲージ
	std::unique_ptr<PlayerGauge> hipDropGauge_;

	// シェイクフラグ
	bool isShake_ = false;
	int shakeFrames_ = 0;
	int currentFrame_ = 0;
	float magnitude_ = 0.0f;
	Vector3 originalPos_;

	// 木のアニメーション用
	float t_ = 0.0f;
	const float kTimer = 2.0f;
	float timer_ = 0.0f;
};
