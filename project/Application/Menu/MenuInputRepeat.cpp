#include "MenuInputRepeat.h"
#include <algorithm>
#include <cmath>

void MenuInputRepeat::Reset(bool waitForRelease) {
	direction_ = 0;
	remaining_ = 0.0f;
	waitForRelease_ = waitForRelease;
}

int MenuInputRepeat::Update(int direction, float deltaTime, float delay, float interval) {
	direction = std::clamp(direction, -1, 1);
	if (waitForRelease_) {
		if (direction == 0)
			waitForRelease_ = false;
		return 0;
	}

	if (direction == 0) {
		Reset();
		return 0;
	}

	if (direction != direction_) {
		direction_ = direction;
		remaining_ = std::max(delay, 0.0f);
		return direction;
	}

	if (!std::isfinite(deltaTime) || deltaTime < 0.0f) {
		return 0;
	}

	remaining_ -= std::min(deltaTime, 1.0f);
	if (remaining_ > 0.0f) {
		return 0;
	}

	const float safeInterval = std::max(interval, 0.01f);

	// フレーム時間が揺れても間隔を保ち、大きな停止の後だけ連続回数を制限する
	const int count = 1 + static_cast<int>(std::floor(-remaining_ / safeInterval));
	remaining_ += static_cast<float>(count) * safeInterval;
	return direction * std::min(count, 8);
}
