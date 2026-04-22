#pragma once
#include "BaseScene.h"
#include "Object3d.h"
#include "Particle.h"
#include "Sprite.h"
#include "AudioManager.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameObjects/Enemy/EnemyBulletManager.h"
#include "GameObjects/Wall/WallManager.h"
#include "GameObjects/Goal/Goal.h"
#include "GameObjects/Enemy/EnemyManager.h"
#include "GameObjects/Player/PlayerHPGauge.h"
#include <array>
#include <memory>
#include <vector>

class GamePlayScene : public BaseScene {
public:
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, Camera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	// 当たり判定
	void CollisionGameObjects();

private:
	// プレイヤー
	std::unique_ptr<Player> player_;

	// 敵
	std::unique_ptr<EnemyManager> enemyManager_;

	// 敵の弾を管理するクラス
	std::unique_ptr<EnemyBulletManager> enemyBulletManager_;

	// 壁の管理クラス
	std::unique_ptr<WallManager> wallManager_;

	// ゴール判定用クラス
	std::unique_ptr<Goal> goal_;

	// 死亡パーティクルリスト
	std::list<std::unique_ptr<TinyEngine::Particle>> enemyDeathParticle_;

	// プレイヤーのHPゲージ
	std::unique_ptr<PlayerHPGauge> playerHPGauge_;

	// デバッグカメラ
	std::unique_ptr<Camera> debugCamera_;
};
