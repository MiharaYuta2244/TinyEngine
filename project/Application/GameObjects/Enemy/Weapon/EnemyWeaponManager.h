#pragma once
#include <list>
#include <memory>
#include <vector>

class EnemyBomb;
class EnemyBullet;

/// <summary>
/// 敵の弾と爆弾を管理するクラス
/// </summary>
class EnemyWeaponManager {
public:
	EnemyWeaponManager();
	~EnemyWeaponManager();

	// 爆弾の更新処理
	void UpdateBombs(float deltaTime);

	// 弾の更新処理
	void UpdateBullets(float deltaTime);

	// 描画処理
	void Draw();

	// 弾の追加
	void AddBullet(std::unique_ptr<EnemyBullet> bullet);

	// 爆弾の追加
	void AddBomb(std::unique_ptr<EnemyBomb> bomb);

	const std::list<std::unique_ptr<EnemyBullet>>& GetBullets() const { return bullets_; }
	const std::vector<std::unique_ptr<EnemyBomb>>& GetBombs() const { return bombs_; }

private:
	std::list<std::unique_ptr<EnemyBullet>> bullets_;
	std::vector<std::unique_ptr<EnemyBomb>> bombs_;
};
