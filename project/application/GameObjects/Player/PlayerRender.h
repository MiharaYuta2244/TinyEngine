#pragma once
#include "Object3d.h"
#include "input/DirectInput.h"
#include <memory>

namespace Player {
class Render {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, DirectInput* input);

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

private:
	void Move(float deltaTime);

private:
	// プレイヤーモデル
	std::unique_ptr<TinyEngine::Object3d> model_;

	// Transform
	Transform transform_;

	// 入力
	DirectInput* input_;

	// モデルの回転スピード
	float rotateSpeed_ = 2.0f;

	Vector2 Velocity_ = {1.0f,1.0f};
};
} // namespace Player