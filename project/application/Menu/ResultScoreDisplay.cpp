#include "ResultScoreDisplay.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

using namespace TinyEngine;

void ResultScoreDisplay::Initialize(EngineContext* ctx, bool isClear, float clearTime, int killCount) {
	// ワイプアニメ開始タイマー
	wipeAnimStartTimer_.Initialize(1.0f);

	// フォント生成
	font_ = std::make_unique<Font>();
	font_->Initialize(ctx, kFontFamily, kFontPixelHeight, kCharSet);

	// 表示文字列の作成
	std::wstring timeStr;
	std::wstring killStr;

	if (isClear) {
		int minutes = std::min(static_cast<int>(clearTime) / 60, 99); // 分は99まで
		int seconds = static_cast<int>(clearTime) % 60;

		std::wostringstream ts;
		ts << std::setfill(L'0') << std::setw(2) << minutes << L':' << std::setw(2) << seconds;
		timeStr = ts.str();

		int clamped = std::min(std::max(killCount, 0), 999); // 3桁上限
		std::wostringstream ks;
		ks << std::setfill(L'0') << std::setw(3) << clamped << L"体";
		killStr = ks.str();
	} else {
		// ゲームオーバー時
		timeStr = L"--:--";
		killStr = L"-体";
	}

	// タイム表示
	timeText_ = std::make_unique<TextSprite>();
	timeText_->Initialize(ctx, font_.get(), timeStr);
	timeText_->SetPosition(kTimeBasePos);

	// キル数表示
	killCountText_ = std::make_unique<TextSprite>();
	killCountText_->Initialize(ctx, font_.get(), killStr);
	killCountText_->SetPosition(kKillCountBasePos);

	// ワイプアニメーションの初期化
	Vector2 wipeSize = {350.0f, 100.0f};

	timerWipe_ = std::make_unique<SpriteScaleWipeAnimator>();
	timerWipe_->Initialize(ctx, kTimeBasePos, wipeSize);

	killCountWipe_ = std::make_unique<SpriteScaleWipeAnimator>();
	killCountWipe_->Initialize(ctx, kKillCountBasePos, wipeSize);

	// タイマー側のワイプを即座に開始
	timerWipe_->StartAnimation();
}

void ResultScoreDisplay::Update(float deltaTime) {
	// ワイプ関連
	wipeAnimStartTimer_.Update(deltaTime);

	if (wipeAnimStartTimer_.IsEnd()) {
		timerWipe_->Update(deltaTime);

		if (!isKillCountWipeStarted_) {
			killCountWipeDelay_ -= deltaTime;
			if (killCountWipeDelay_ <= 0.0f) {
				killCountWipe_->StartAnimation();
				isKillCountWipeStarted_ = true;
			}
		} else {
			killCountWipe_->Update(deltaTime);
		}
	}

	// テキストの更新
	timeText_->Update();
	killCountText_->Update();
}

void ResultScoreDisplay::Draw() {
	// クリアタイム描画
	if (timerWipe_->GetBeforeAnimationFinished()) {
		timeText_->Draw();
	}
	timerWipe_->Draw();

	// キル数描画
	if (killCountWipe_->GetBeforeAnimationFinished()) {
		killCountText_->Draw();
	}
	killCountWipe_->Draw();
}