#pragma once
#include "Enemy.h"
#include <list>
#include <memory>
#include <string>

class Player;
class EnemyBulletManager;
class WallManager;

/// <summary>
/// 敵管理クラス
/// </summary>
class EnemyManager {
public:
	void Initialize(EngineContext* ctx);
	void Update(float deltaTime, Player* player, EnemyBulletManager* enemyBulletManager, WallManager* wallManager);
	void PostUpdate();
	void Draw();
	void DrawImGui();

	// 敵のリストを取得するGetter
	std::list<std::unique_ptr<Enemy>>& GetEnemies() { return enemies_; }

private:
	void LoadFromCSV(const std::string& filepath);
	void SaveToCSV(const std::string& filepath);

private:
	EngineContext* ctx_ = nullptr;
	std::string csvPath_;

	// 敵オブジェクトのリスト
	std::list<std::unique_ptr<Enemy>> enemies_;
};