#pragma once
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "Rect.h"

/// <summary>
/// 敵の弾クラス
/// </summary>
class EnemyBullet {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, Vector2 dir, Vector3 pos);

	// 更新処理
	void Update(float deltaTime, float bulletSpeed);

	// 描画処理
	void Draw();

	// 削除用の関数
	bool IsDead(const Rect<float>& rect) const;

	// 座標のGetter
	Vector3 GetPosition() const { return transform_.translate; }

private:
	Transform transform_;

	// 描画用インスタンス
	std::unique_ptr<ObjectRender> render_;

	// 移動方向
	Vector2 direction_;
};
