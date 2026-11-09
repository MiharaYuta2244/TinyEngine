#include "ThrowPreview.h"
#include "MathUtility.h"
#include <cmath>
#include <numbers>

using namespace TinyEngine;

void ThrowPreview::Initialize(DecalManager* decalManager) {
	const Vector3 flatRotate = {std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f};

	for (auto& decal : decals_) {
		// 描画順を100にして、他のデカールより手前に出す
		decal = decalManager->AddDecal("white.png", {0.0f, kHiddenY, 0.0f}, flatRotate, {0.15f, 0.15f, 1.0f}, {color_.x, color_.y, color_.z, 0.0f}, 0.0f, 0.0f, 100);
	}

	scrollOffset_ = 0.0f;
}

void ThrowPreview::Hide() {
	for (auto& decal : decals_) {
		if (decal) {
			// 透明＆画面外に
			decal->color.w = 0.0f;
			decal->transform.translate.y = kHiddenY;
		}
	}

	// 非表示中はスクロール位置をリセット
	scrollOffset_ = 0.0f;
}

void ThrowPreview::Update(float deltaTime, const Vector3& startPos, const Vector2& dir, float power, float friction) {
	// 方向の正規化
	float dirLen = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	if (dirLen < 0.0001f) {
		Hide();
		return;
	}

	Vector2 nd = {dir.x / dirLen, dir.y / dirLen};

	// 軌道シミュレーション
	std::vector<Vector3> points = SimulateTrajectory(startPos, {nd.x * power, nd.y * power}, friction);
	if (points.empty()) {
		Hide();
		return;
	}

	// 各点の累積距離
	std::vector<float> distances;
	float totalDistance = BuildAccumulatedDistances(points, distances);
	if (totalDistance <= 0.0001f) {
		Hide();
		return;
	}

	// スクロール処理
	scrollOffset_ += scrollSpeed_ * deltaTime;
	scrollOffset_ = std::fmod(scrollOffset_, totalDistance);

	// 各ドットを配置
	for (int i = 0; i < kDotCount; ++i) {
		// 等間隔に並べた基準位置+スクロール分
		float baseDist = totalDistance * (static_cast<float>(i) / static_cast<float>(kDotCount - 1));
		float targetDist = std::fmod(baseDist + scrollOffset_, totalDistance);

		Vector3 dotPos = SampleTrajectory(points, distances, targetDist);
		dotPos.y = kDotHeight;
		decals_[i]->transform.translate = dotPos;

		// 先頭ほど大きく、後ろほど小さく
		float sizeT = targetDist / totalDistance;
		float dotScale = MathUtility::Lerp(kMaxDotScale, kMinDotScale, sizeT);
		decals_[i]->transform.scale = {dotScale, dotScale, 1.0f};

		decals_[i]->color = color_;
	}
}

std::vector<Vector3> ThrowPreview::SimulateTrajectory(Vector3 pos, Vector2 velocity, float friction) const {
	std::vector<Vector3> points;
	points.reserve(kMaxSimSteps);

	for (int step = 0; step < kMaxSimSteps; ++step) {
		pos.x += velocity.x * kSimDeltaTime;
		pos.z += velocity.y * kSimDeltaTime;

		// 減衰
		velocity.x = MathUtility::Lerp(velocity.x, 0.0f, friction * kSimDeltaTime);
		velocity.y = MathUtility::Lerp(velocity.y, 0.0f, friction * kSimDeltaTime);

		const bool stopped = std::abs(velocity.x) < kStopSpeed && std::abs(velocity.y) < kStopSpeed;
		if (stopped) {
			velocity = {0.0f, 0.0f};
		}

		points.push_back(pos);

		if (stopped) {
			break;
		}
	}

	return points;
}

float ThrowPreview::BuildAccumulatedDistances(const std::vector<Vector3>& points, std::vector<float>& outDistances) {
	outDistances.clear();
	outDistances.reserve(points.size());
	outDistances.push_back(0.0f);

	float total = 0.0f;
	for (size_t i = 1; i < points.size(); ++i) {
		float dx = points[i].x - points[i - 1].x;
		float dz = points[i].z - points[i - 1].z;
		total += std::sqrt(dx * dx + dz * dz);
		outDistances.push_back(total);
	}

	return total;
}

Vector3 ThrowPreview::SampleTrajectory(const std::vector<Vector3>& points, const std::vector<float>& distances, float distance) {
	Vector3 result = points.front();

	for (size_t j = 1; j < points.size(); ++j) {
		if (distances[j] < distance) {
			continue;
		}

		float segmentLen = distances[j] - distances[j - 1];
		if (segmentLen > 0.0001f) {
			float t = (distance - distances[j - 1]) / segmentLen;
			result.x = points[j - 1].x + (points[j].x - points[j - 1].x) * t;
			result.z = points[j - 1].z + (points[j].z - points[j - 1].z) * t;
		} else {
			result = points[j];
		}

		break;
	}

	return result;
}