#include "MeteoriteGenerator.h"

void MeteoriteGenerator::Initialize(EngineContext* ctx) { ctx_ = ctx; }

void MeteoriteGenerator::Update(float dt) {
	// 経過時間の記録
	dt_ = dt;

	// 隕石の生成&初期化
	GenerateMeteorites();

	// 隕石の更新
	for (auto& meteorite : meteorites_) {
		if (meteorite) {
			meteorite->Update(dt);
		}
	}

	// 隕石の削除
	EraseMeteorites();
}

void MeteoriteGenerator::Draw() {
	// 隕石の描画
	for (auto& meteorite : meteorites_) {
		if (meteorite) {
			meteorite->Draw();
		}
	}
}

void MeteoriteGenerator::GenerateMeteorites() {
	// 時間の加算
	AddGenerateTimer();

	// 隕石の生成
	if (currentTimer_ >= kMaxGenerateTimer) {
		for (int i = 0; i < meteorites_.size(); ++i) {
			meteorites_[i] = std::make_unique<Meteorite>();
			meteorites_[i]->Initialize(ctx_);
			meteorites_[i]->SetTranslate({i * 3.0f, 5.0f, 0.0f});
		}
	}
}

void MeteoriteGenerator::AddGenerateTimer() {
	// 時間の加算
	currentTimer_ += dt_;
}

void MeteoriteGenerator::EraseMeteorites() {
	// 隕石の削除
	for (auto& m : meteorites_) {
		if (m && m->GetEndAnimation()) {
			m.reset(); // 実質的な削除
		}
	}
}
