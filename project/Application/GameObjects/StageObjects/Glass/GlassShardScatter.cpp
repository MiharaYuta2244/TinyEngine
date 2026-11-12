#include "GlassShardScatter.h"
#include "ColorPalette.h"
#include "Easing.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include "Random.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace TinyEngine;

void GlassShardScatter::Initialize(DecalManager* decalManager) {
	decalManager_ = decalManager;
	shards_.clear();
}

void GlassShardScatter::Spawn(const Vector3& center, const Vector3& halfExtent, const Vector3& hitDir) {
	const float pi = std::numbers::pi_v<float>;

	// 叩いた方向。ゼロなら全方向に散らす
	Vector3 dir = {hitDir.x, 0.0f, hitDir.z};
	const bool hasDir = MathUtility::LengthSquared(dir) > 0.0001f;
	const float baseAngle = hasDir ? std::atan2(dir.x, dir.z) : 0.0f;

	for (int i = 0; i < shardCount_; ++i) {
		Shard s;

		// 出発点はガラス面のどこか
		s.start = {center.x + RandomUtils::RangeFloat(-halfExtent.x, halfExtent.x), 0.05f, center.z + RandomUtils::RangeFloat(-halfExtent.z, halfExtent.z)};

		// 飛ぶ方向。前方扇状と、周囲への飛び散りを混ぜる
		float angle;
		float distRate = RandomUtils::RangeFloat(0.0f, 1.0f);

		if (hasDir && RandomUtils::Chance(forwardRatio_)) {
			// 中心寄りに集まる分布で、扇の端ほど少なくする
			float r = RandomUtils::RangeFloat(-1.0f, 1.0f);
			angle = baseAngle + r * std::abs(r) * coneHalfAngle_;
			distRate = std::pow(distRate, 1.5f); // 遠くまで飛ぶ破片は少なめ
		} else {
			angle = RandomUtils::RangeFloat(0.0f, 2.0f * pi);
			distRate = std::pow(distRate, 3.0f) * 0.5f; // 周囲は短く
		}

		float dist = 0.5f + distRate * maxDistance_;
		s.end = {s.start.x + std::sin(angle) * dist, 0.05f + RandomUtils::RangeFloat(0.0f, 0.04f), s.start.z + std::cos(angle) * dist};

		// 大きさは小さい破片が多く、大きい破片は少なく。細長くして飛ぶ方向へ向ける
		float size = minSize_ + (maxSize_ - minSize_) * std::pow(RandomUtils::RangeFloat(0.0f, 1.0f), 3.0f);
		s.baseScale = {size * RandomUtils::RangeFloat(0.5f, 1.0f), size * RandomUtils::RangeFloat(1.0f, 2.2f), 1.0f};

		// ローカルYが進行方向を向くyaw
		s.yaw = angle + RandomUtils::RangeFloat(-0.5f, 0.5f);
		s.spin = RandomUtils::RangeFloat(-6.0f, 6.0f);

		// 遠くへ飛ぶほど少し長く飛ぶ。小さい破片ほど速く着地する
		s.duration = 0.12f + dist * 0.05f;

		// 少し白〜水色に揺らす
		float tint = RandomUtils::RangeFloat(0.0f, 1.0f);
		Vector4 color = {0.75f + 0.25f * tint, 0.9f + 0.1f * tint, 1.0f, RandomUtils::RangeFloat(0.6f, 1.0f)};

		const Vector3 rotate = {pi / 2.0f, s.yaw + s.spin, 0.0f};
		s.decal = decalManager_->AddDecal("Shard.png", s.start, rotate, s.baseScale * 0.5f, color, 10.0f, 3.0f);
		shards_.push_back(s);
	}
}

void GlassShardScatter::Update(float deltaTime) {
	const float pi = std::numbers::pi_v<float>;

	for (auto& s : shards_) {
		s.elapsed += deltaTime;
		float t = std::clamp(s.elapsed / s.duration, 0.0f, 1.0f);

		// 勢いよく飛び出して、急に減速する
		float e = Easing::ApplyEasing(EaseType::EASEOUTEXPO, t);

		Transform& tr = s.decal->transform;
		tr.translate = MathUtility::Lerp(s.start, s.end, e);

		// 飛行中は回転し、着地するにつれて最終の向きに収束
		tr.rotate = {pi / 2.0f, s.yaw + s.spin * (1.0f - e), 0.0f};

		// 飛び出しは小さく、着地で本来のサイズへ
		float k = 0.5f + 0.5f * e;
		tr.scale = {s.baseScale.x * k, s.baseScale.y * k, 1.0f};
	}

	// 着地し終わった破片は管理対象から外す
	std::erase_if(shards_, [](const Shard& s) { return s.elapsed >= s.duration; });
}