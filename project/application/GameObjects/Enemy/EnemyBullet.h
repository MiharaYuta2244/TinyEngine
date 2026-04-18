#pragma once
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "Rect.h"

class EnemyBullet {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, Vector2 dir);

	// 更新処理
	void Update(float deltaTime, float bulletSpeed);

	// 描画処理
	void Draw();

	// 削除用の関数
	bool IsDead(const Rect<float>& rect) const;

private:
	Transform transform_;

	// 描画用インスタンス
	std::unique_ptr<ObjectRender> render_;

	// 移動方向
	Vector2 direction_;
};
