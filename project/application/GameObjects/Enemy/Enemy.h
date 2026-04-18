#pragma once
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "EnemyAI.h"

class Enemy {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, Vector3 playerPos);

	// 描画処理
	void Draw();

	// 死亡フラグGetter
	bool IsDead() const;

private:
	Transform transform_;

	std::unique_ptr<ObjectRender> render_; // 描画用インスタンス
	std::unique_ptr<EnemyAI> ai_; // AI
};
