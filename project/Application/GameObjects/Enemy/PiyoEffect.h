#pragma once
#include "GameObjects/ObjectRender/ObjectRender.h"
#include <array>
#include <memory>

/// <summary>
/// ダウン中の敵の頭上で星が回るピヨピヨ演出
/// </summary>
class PiyoEffect {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(float deltaTime, const Vector3& enemyPos);

	// 描画処理
	void Draw();

private:
	static constexpr int kStarCount = 3;
	std::array<std::unique_ptr<ObjectRender>, kStarCount> renders_;

	float angle_ = 0.0f;        // 現在の回転角
	float angularSpeed_ = 6.0f; // 回転速度
	float radius_ = 1.0f;       // 回る半径
	float height_ = 2.0f;       // 頭上の高さ
	float starSize_ = 0.2f;     // 星のサイズ
};