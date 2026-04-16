#pragma once
#include "Object3d.h"
#include <memory>

class Player {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

private:
	// プレイヤーモデル
	std::unique_ptr<TinyEngine::Object3d> model_;

	// Transform
	Transform transform_;
};
