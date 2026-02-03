#pragma once
#include "Meteorite.h"
#include <array>

class MeteoriteGenerator {
public:
	void Initialize(EngineContext* ctx);

	void Update(float dt);

	void Draw();

private:
	// 隕石の生成
	void GenerateMeteorites();

	// タイマーの加算
	void AddGenerateTimer();

	// 隕石の削除
	void EraseMeteorites();

private:
	// 生成する隕石の配列
	std::array<std::unique_ptr<Meteorite>, 4> meteorites_;

	// 経過時間
	float dt_=0.0f;

	// 現在の経過時間
	float currentTimer_ = 0.0f;

	// 生成での時間
	const float kMaxGenerateTimer = 5.0f;

	// エンジン機能
	EngineContext* ctx_;
};
