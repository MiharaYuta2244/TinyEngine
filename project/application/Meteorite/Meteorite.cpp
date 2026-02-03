#include "Meteorite.h"

void Meteorite::Initialize(EngineContext* ctx) {
	// モデルの生成&初期化
	meteoriteModel_ = std::make_unique<Object3d>();
	meteoriteModel_->Initialize(ctx);
	meteoriteModel_->SetModel("Box.obj");
	meteoriteModel_->SetScale({3.0f, 3.0f, 3.0f});

	// 落下アニメーションの初期設定
	SettingFallAnimation();
}

void Meteorite::Update(float dt) {
	// 経過時間
	dt_ = dt;

	// 落下
	Fall();

	// モデルの更新
	if (meteoriteModel_) {
		meteoriteModel_->Update();
	}
}

void Meteorite::Draw() {
	// モデルの描画
	if (meteoriteModel_) {
		meteoriteModel_->Draw();
	}
}

void Meteorite::Fall() {
	// 落下アニメーション
	if (fallAnimation_.anim.GetIsActive()) {
		// アニメーションの更新
		fallAnimation_.anim.Update(dt_, fallAnimation_.temp);
		meteoriteModel_->SetTranslate({meteoriteModel_->GetTranslate().x, fallAnimation_.temp, meteoriteModel_->GetTranslate().z});
	}
}

void Meteorite::SettingFallAnimation() {
	// 落下アニメーションの初期設定
	fallAnimation_.anim = {meteoriteModel_->GetTranslate().y, 0.0f, 1.0f, EaseType::EASEINBACK};
}
