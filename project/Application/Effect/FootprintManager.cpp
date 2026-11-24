#include "FootprintManager.h"
#include <numbers>

using namespace TinyEngine;

void FootprintManager::Initialize(DecalManager* decalManager) {
	pool_.resize(kPoolSize);

	// 最初に全部作って画面外に隠しておく
	for (auto& fp : pool_) {
		fp.decal = decalManager->AddDecal("Footprint.png", {0.0f, -1000.0f, 0.0f}, {std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f}, scale_, {baseColor_.x, baseColor_.y, baseColor_.z, 0.0f});
		fp.active = false;
	}
}

void FootprintManager::Update(float deltaTime) {
	for (auto& fp : pool_) {
		if (!fp.active) {
			continue;
		}

		fp.timer += deltaTime;

		if (fp.timer >= holdTime_ + fadeTime_) {
			// 寿命が尽きたので隠してプールに戻す
			fp.decal->color.w = 0.0f;
			fp.decal->transform.translate.y = -1000.0f;
			fp.active = false;
			continue;
		}

		float alpha = maxAlpha_;
		if (fp.timer > holdTime_) {
			float t = (fp.timer - holdTime_) / fadeTime_;
			alpha = maxAlpha_ * (1.0f - t);
		}
		fp.decal->color.w = alpha;
	}
}

void FootprintManager::Add(const Vector3& pos, float yaw, bool isLeft) {
	// リングバッファ式に一番古いものを上書きする
	Footprint& fp = pool_[nextIndex_];
	nextIndex_ = (nextIndex_ + 1) % pool_.size();

	Transform& t = fp.decal->transform;
	t.translate = {pos.x, height_, pos.z};
	t.rotate = {std::numbers::pi_v<float> / 2.0f, yaw, 0.0f};

	// 右足テクスチャ1枚を左右反転して左足にする
	t.scale = {isLeft ? -scale_.x : scale_.x, scale_.y, scale_.z};

	fp.decal->color = baseColor_;
	fp.decal->color.w = maxAlpha_;

	fp.timer = 0.0f;
	fp.active = true;
}