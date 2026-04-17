#pragma once
#include "Object3d.h"
#include <memory>

/// <summary>
/// プレイヤーの描画処理をまとめたクラス
/// </summary>
class PlayerRender {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx);

	// 更新処理
	void Update(Transform transform);

	// 描画処理
	void Draw();

private:
	// プレイヤーモデル
	std::unique_ptr<TinyEngine::Object3d> object3d_;
};