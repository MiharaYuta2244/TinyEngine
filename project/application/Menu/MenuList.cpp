#include "MenuList.h"
#include "MathOperator.h"

using namespace TinyEngine;

void MenuList::Initialize(EngineContext* ctx, const std::wstring& fontFamily, int fontPixelHeight, bool useKanji) {
	ctx_ = ctx;

	// フォント生成
	font_ = std::make_unique<TinyEngine::Font>();
	font_->Initialize(ctx_, fontFamily, fontPixelHeight, TinyEngine::Font::MakeJapaneseCharSet(useKanji));

	// 決定ボタン入力時のエフェクト
	decideEffect_ = std::make_unique<DecideEffect>();
	decideEffect_->Initialize(ctx, startPos_);

	// オーディオマネージャー生成&初期化
	audioManager_ = std::make_unique<AudioManager>();
	audioManager_->Initialize();
	audioManager_->LoadWave("Select", "resources/sounds/se/Select.mp3");
	audioManager_->LoadWave("Decide", "resources/sounds/se/Decide.mp3");

	// 選択中背景スプライトの生成&初期化
	selectorBg_ = std::make_unique<Sprite>();
	selectorBg_->Initialize(ctx_, "white.png");
	selectorBg_->SetColor({1.0f, 1.0f, 1.0f, 0.2f});
	selectorBg_->SetSize(selectorBgSize_);
	selectorBg_->SetAnchorPoint({0.5f, 0.0f});

	// スクロールバーの生成&初期化
	scrollTrack_ = std::make_unique<Sprite>();
	scrollTrack_->Initialize(ctx_, "white.png");
	scrollTrack_->SetColor(scrollTrackColor_);
	scrollTrack_->SetAnchorPoint({0.0f, 0.0f});

	scrollThumb_ = std::make_unique<Sprite>();
	scrollThumb_->Initialize(ctx_, "white.png");
	scrollThumb_->SetColor(scrollThumbColor_);
	scrollThumb_->SetAnchorPoint({0.0f, 0.0f});
}

void MenuList::AddItem(const std::wstring& label, std::function<void()> onSelect) {
	MenuItem item;
	item.label = label;
	item.onSelect = onSelect;
	item.text = std::make_unique<TinyEngine::TextSprite>();
	item.text->Initialize(ctx_, font_.get(), label);
	item.text->SetAnchorPoint({0.5f, 0.5f});
	item.text->SetScale(textScale_);
	items_.push_back(std::move(item));
}

void MenuList::AddToggleItem(const std::wstring& label, bool* target) {
	MenuItem item;
	item.label = label;
	item.toggleValue = target;
	item.onSelect = [target]() { *target = !(*target); };

	item.text = std::make_unique<TinyEngine::TextSprite>();
	item.text->Initialize(ctx_, font_.get(), label);
	item.text->SetAnchorPoint({0.5f, 0.5f});
	item.text->SetScale(textScale_);

	item.checkIcon = std::make_unique<TinyEngine::Sprite>();
	item.checkIcon->Initialize(ctx_, "Check.png");

	items_.push_back(std::move(item));
}

void MenuList::Update(DirectInput* input, GamePad* gamePad, float deltaTime) {
	int index = static_cast<int>(items_.size());

	// スティック制御用クールダウン
	stickCooldown_ -= deltaTime;

	// ボタンの選択
	bool up = input->KeyTriggered(DIK_W);
	bool down = input->KeyTriggered(DIK_S);
	bool decide = input->KeyTriggered(DIK_SPACE);

	// パッド入力
	if (gamePad && gamePad->GetState().connected) {
		const auto& pad = gamePad->GetState();

		float ly = pad.axes.ly;

		// スティックが一定以上倒されたら
		if (!stickInUse_) {
			if (ly < -0.9f) {
				up = true;
				stickInUse_ = true;
			} else if (ly > 0.9f) {
				down = true;
				stickInUse_ = true;
			}
		}

		// スティックがニュートラルに戻ったら再入力可能に
		if (std::abs(ly) < 0.3f) {
			stickInUse_ = false;
		}

		if (pad.buttonsPressed.a) {
			decide = true;
		}

		if (pad.buttonsPressed.dpadUp) {
			up = true;
		}

		if (pad.buttonsPressed.dpadDown) {
			down = true;
		}
	}

	// 決定アニメーション再生中は他の項目へ移動させない
	if (up && !isDecided_) {
		currentIndex_ = (currentIndex_ - 1 + index) % index;
		audioManager_->PlaySE("Select", 0.5f);
	}
	if (down && !isDecided_) {
		currentIndex_ = (currentIndex_ + 1) % index;
		audioManager_->PlaySE("Select", 0.5f);
	}

	// 決定処理
	if (decide && !isDecided_) {
		items_[currentIndex_].onSelect();
		audioManager_->PlaySE("Decide", 0.5f);

		if (items_[currentIndex_].toggleValue) {
			// トグル項目はロックせず即座に再入力可能にする
		} else {
			decideEffect_->StartAnimation();
			isDecided_ = true;
		}
	}

	// 選択が変わったら揺れ開始
	if (currentIndex_ != prevIndex_) {
		if (!items_.empty()) {
			items_[currentIndex_].wiggleTimer = 0.0f;
		}
		prevIndex_ = currentIndex_;
	}

	// 揺れタイマー更新
	for (auto& item : items_) {
		if (item.wiggleTimer < wiggleDuration_) {
			item.wiggleTimer += deltaTime;
		}
	}

	// スクロール更新
	UpdateScroll(deltaTime);
	UpdateScrollBar();

	// 背景スプライトの座標を選択中アイテムに合わせる
	selectorBg_->SetPosition({startPos_.x, GetRowY(currentIndex_)});
	selectorBg_->Update();

	// 決定ボタン入力時エフェクト更新
	decideEffect_->Update(deltaTime);

	// 選択中メニューの座標をエフェクトに適用
	decideEffect_->SetPos({startPos_.x, GetRowY(currentIndex_)});

	// 音声更新
	audioManager_->Update();
}

void MenuList::Draw() {
	// 背景
	selectorBg_->Draw();

	// 決定時エフェクト
	decideEffect_->Draw();

	for (int i = 0; i < items_.size(); i++) {
		// 表示範囲外は描画しない
		if (!IsRowVisible(i)) {
			continue;
		}

		auto& item = items_[i];
		auto& text = item.text;

		text->SetColor(i == currentIndex_ ? selectColor_ : normalColor_);

		// 左上基準の座標にテキストサイズの半分を足す
		Vector2 size = text->GetTextSize();
		text->SetPosition({startPos_.x + size.x * textScale_ * 0.5f, GetRowY(i) + size.y * textScale_ * 0.5f});

		// 揺れ
		float angle = 0.0f;
		if (item.wiggleTimer < wiggleDuration_) {
			float t = item.wiggleTimer / wiggleDuration_;
			angle = wiggleAngle_ * std::sin(t * wiggleCycles_ * 2.0f * std::numbers::pi_v<float>) * (1.0f - t);
		}

		text->SetRotation(angle);
		text->Update();
		text->Draw();

		// トグル項目ならON/OFF状態のアイコンも描画
		if (item.toggleValue && item.checkIcon) {
			bool isOn = *item.toggleValue;
			item.checkIcon->SetColor(isOn ? Vector4{0.3f, 1.0f, 0.3f, 1.0f} : Vector4{0.4f, 0.4f, 0.4f, 0.5f});
			item.checkIcon->SetPosition({startPos_.x + checkIconOffsetX_, GetRowY(i)});
			item.checkIcon->Update();
			item.checkIcon->Draw();
		}
	}

	// スクロールバー
	if (HasScrollBar()) {
		scrollTrack_->Draw();
		scrollThumb_->Draw();
	}
}

bool MenuList::IsRowVisible(int index) const {
	if (maxVisibleCount_ <= 0) {
		return true;
	}

	const float row = static_cast<float>(index) - scrollOffset_;
	const float eps = 0.01f;

	return row >= -eps && row <= static_cast<float>(maxVisibleCount_ - 1) + eps;
}

void MenuList::UpdateScroll(float deltaTime) {
	if (maxVisibleCount_ <= 0) {
		scrollOffset_ = 0.0f;
		return;
	}

	const int count = static_cast<int>(items_.size());
	const int maxTop = std::max(0, count - maxVisibleCount_);

	// 選択項目が範囲内に収まるように先頭を動かす
	if (currentIndex_ < scrollTop_) {
		scrollTop_ = currentIndex_;
	} else if (currentIndex_ >= scrollTop_ + maxVisibleCount_) {
		scrollTop_ = currentIndex_ - maxVisibleCount_ + 1;
	}

	scrollTop_ = std::clamp(scrollTop_, 0, maxTop);

	// 追従
	const float t = std::min(1.0f, scrollSpeed_ * deltaTime);
	scrollOffset_ += (static_cast<float>(scrollTop_) - scrollOffset_) * t;
}

void MenuList::UpdateScrollBar() {
	if (!HasScrollBar()) {
		return;
	}

	const int count = static_cast<int>(items_.size());
	const int maxTop = count - maxVisibleCount_;

	// トラックは表示範囲いっぱいの高さ
	const float trackX = startPos_.x + scrollBarOffsetX_;
	const float trackY = startPos_.y;
	const float trackH = offsetY_ * static_cast<float>(maxVisibleCount_) - 10.0f;

	// つまみの長さは表示数/全項目数の割合
	const float ratio = static_cast<float>(maxVisibleCount_) / static_cast<float>(count);
	const float thumbH = std::max(trackH * ratio, scrollBarMinThumbHeight_);

	// つまみの位置は補間後のスクロール量に連動
	const float progress = std::clamp(scrollOffset_ / static_cast<float>(maxTop), 0.0f, 1.0f);
	const float thumbY = trackY + (trackH - thumbH) * progress;

	scrollTrack_->SetPosition({trackX, trackY});
	scrollTrack_->SetSize({scrollBarWidth_, trackH});
	scrollTrack_->Update();

	scrollThumb_->SetPosition({trackX, thumbY});
	scrollThumb_->SetSize({scrollBarWidth_, thumbH});
	scrollThumb_->Update();
}