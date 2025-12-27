#include "MathOperator.h"
#include "MathUtility.h"
#include "StageModel.h"
#include <numbers>

void StageModel::Initialize(EngineContext* ctx) {
	stageModel_ = std::make_unique<Object3d>();
	stageModel_->Initialize(ctx);
	stageModel_->SetModel("start.obj");
	stageModel_->SetTranslate({20.0f, 2.0f, 0.0f});
	stageModel_->SetScale(baseScale_);
	stageModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
}

void StageModel::Update(float deltaTime) {
	// 拡縮アニメーション
	Animation(deltaTime);

	if (!isSelected_) {
		stageModel_->SetScale(baseScale_ * 0.5f); // 非選択状態のときは通常よりサイズを小さくする
		animationTime_ = 0.0f;                    // アニメーションタイムをリセット
	}

	stageModel_->Update();
}

void StageModel::Draw() { stageModel_->Draw(); }

void StageModel::Animation(float deltaTime) {
	if (!isSelected_)
		return;

	animationTime_ += deltaTime * animationSpeed_;

	// サイン波で拡縮を繰り返す（0～1を往復）
	float t = (sin(animationTime_) + 1.0f) * 0.5f;

	// 補間して新しいスケールを計算
	Vector3 newScale = MathUtility::Lerp(baseScale_, targetScale_, t);

	stageModel_->SetScale(newScale);
}
