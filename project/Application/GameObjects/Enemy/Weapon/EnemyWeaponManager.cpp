#include "EnemyWeaponManager.h"
#include "EnemyBomb.h"
#include "EnemyBullet.h"
#include <utility>

EnemyWeaponManager::EnemyWeaponManager() = default;
EnemyWeaponManager::~EnemyWeaponManager() = default;

void EnemyWeaponManager::UpdateBombs(float deltaTime) {
	for (auto& bomb : bombs_) {
		bomb->Update(deltaTime);
	}
	std::erase_if(bombs_, [](const std::unique_ptr<EnemyBomb>& bomb) { return bomb->IsFinished(); });
}

void EnemyWeaponManager::UpdateBullets(float deltaTime) {
	for (auto& bullet : bullets_) {
		bullet->Update(deltaTime);
	}
	std::erase_if(bullets_, [](const std::unique_ptr<EnemyBullet>& bullet) { return bullet->IsDead(); });
}

void EnemyWeaponManager::Draw() {
	for (auto& bullet : bullets_) {
		bullet->Draw();
	}
	for (auto& bomb : bombs_) {
		bomb->Draw();
	}
}

void EnemyWeaponManager::AddBullet(std::unique_ptr<EnemyBullet> bullet) {
	bullets_.push_back(std::move(bullet));
}

void EnemyWeaponManager::AddBomb(std::unique_ptr<EnemyBomb> bomb) {
	bombs_.push_back(std::move(bomb));
}
