#include "MenuList.h"
#include "Game/GameAudio.h"
#include "MathOperator.h"
#include <cmath>
#include <numbers>

using namespace TinyEngine;

void MenuList::Initialize(EngineContext* ctx, const std::wstring& fontFamily, int fontPixelHeight, bool useKanji) {
	ctx_ = ctx;

	// フォント生成
	font_ = std::make_unique<TinyEngine::Font>();
	font_->Initialize(ctx_, fontFamily, fontPixelHeight, TinyEngine::Font::MakeJapaneseCharSet(useKanji));

	// 決定ボタン入力時のエフェクト
	decideEffect_ = std::make_unique<DecideEffect>();
	decideEffect_->Initialize(ctx, startPos_);

	// 選択中背景スプライトの生成&初期化
	selectorBg_ = std::make_unique<Sprite>();
	selectorBg_->Initialize(ctx_, "white.png");
	selectorBg_->SetColor({1.0f, 1.0f, 1.0f, 0.2f});
	selectorBg_->SetSize(selectorBgSize_);
	selectorBg_->SetAnchorPoint({0.5f, 0.5f});
	selectorBgAnimationTime_ = kSelectorBgAnimationDuration;

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

void MenuList::ResetState() {
	isDecided_ = false;
	draggingSlider_ = -1;
	navigationRepeat_.Reset(true);
	adjustmentRepeat_.Reset(true);
	verticalStickDirection_ = horizontalStickDirection_ = 0;
}

void MenuList::AddItem(const std::wstring& label, std::function<void()> onSelect, bool keepActive) {
	MenuItem item;
	item.keepActive = keepActive;
	item.label = label;
	item.onSelect = onSelect;
	item.text = std::make_unique<TinyEngine::TextSprite>();
	item.text->Initialize(ctx_, font_.get(), label);
	item.text->SetAnchorPoint({0.5f, 0.5f});
	item.text->SetScale(textScale_);
	CreateRowVisuals(item);
	items_.push_back(std::move(item));
}

void MenuList::AddToggleItem(const std::wstring& label, bool* target) {
	MenuItem item;
	item.label = label;
	item.toggleValue = target;
	item.onSelect = [target]() { *target = !(*target); };
	item.onAdjust = [target](int direction) { *target = direction < 0; };

	item.text = std::make_unique<TinyEngine::TextSprite>();
	item.text->Initialize(ctx_, font_.get(), label);
	item.text->SetAnchorPoint({0.5f, 0.5f});
	item.text->SetScale(textScale_);

	CreateControl(item);

	items_.push_back(std::move(item));
}

void MenuList::AddSliderItem(const std::wstring& label, std::function<float()> value, std::function<void(float)> changed) {
	AddItem(label, []() {}, true);
	auto& item = items_.back();
	item.sliderValue = std::move(value);
	item.setSliderValue = std::move(changed);
	item.onAdjust = [get = item.sliderValue, set = item.setSliderValue](int direction) { set(std::clamp((std::round(get() * 100.0f) + static_cast<float>(direction)) / 100.0f, 0.0f, 1.0f)); };

	CreateControl(item);
}

void MenuList::CreateControl(MenuItem& item) {
	const auto create = [this]() {
		auto sprite = std::make_unique<Sprite>();
		sprite->Initialize(ctx_, "white.png");
		sprite->SetAnchorPoint({0.5f, 0.5f});
		return sprite;
	};

	CreateRowVisuals(item);
	item.track = create();
	item.background = create();
	item.fill = create();
	item.valueText = std::make_unique<TextSprite>();
	item.valueText->Initialize(ctx_, font_.get(), L"");
	item.offText = std::make_unique<TextSprite>();
	item.offText->Initialize(ctx_, font_.get(), L"Off");
}

void MenuList::CreateRowVisuals(MenuItem& item) {
	if (item.panel)
		return;

	item.panel = std::make_unique<Sprite>();
	item.panel->Initialize(ctx_, "white.png");
	item.panel->SetAnchorPoint({0.5f, 0.5f});
	item.edge = std::make_unique<Sprite>();
	item.edge->Initialize(ctx_, "white.png");
	item.edge->SetAnchorPoint({0.5f, 0.5f});
}

void MenuList::DrawRowBackground(MenuItem& item, int index) {
	// 操作部品の有無にかかわらず、同じ幅・行間・選択線で統一する。
	const auto row = GetRowPosition(index);
	const auto colors = layout_ ? layout_->colors : MenuPalette{};
	const float height = std::max(offsetY_ - 4.0f, 1.0f);
	const Vector2 center = {row.x - 20.0f + selectorBgSize_.x * 0.5f, row.y + offsetY_ * 0.5f};
	item.panel->SetPosition(center);
	item.panel->SetSize({selectorBgSize_.x, height});
	item.panel->SetColor(index == currentIndex_ ? colors.selectedRowBackground : colors.rowBackground);
	item.panel->Update();
	item.panel->Draw();

	if (index == currentIndex_) {
		item.edge->SetPosition({center.x, center.y - height * 0.5f + 1.0f});
		item.edge->SetSize({selectorBgSize_.x, 2.0f});
		item.edge->SetColor(colors.accent);
		item.edge->Update();
		item.edge->Draw();
	}
}

void MenuList::DrawControl(MenuItem& item, int index) {
	const auto row = GetRowPosition(index);
	const float x = row.x + checkIconOffsetX_, y = row.y + offsetY_ * 0.5f;
	const float width = std::max(controlSize_.x, controlSize_.y), height = controlSize_.y;
	const auto colors = layout_ ? layout_->colors : MenuPalette{};
	const auto draw = [](Sprite& sprite, Vector2 position, Vector2 size, Vector4 color) {
		sprite.SetPosition(position);
		sprite.SetSize(size);
		sprite.SetColor(color);
		sprite.Update();
		sprite.Draw();
	};

	const float scale = textScale_ * 0.75f;
	if (item.toggleValue) {
		// OnとOffを常に横並びにし、現在の設定だけ選択色で強調する。
		const bool enabled = *item.toggleValue;
		const float gap = std::min(8.0f, width * 0.1f);
		const float buttonWidth = (width - gap) * 0.5f;
		draw(*item.track, {x + buttonWidth * 0.5f, y}, {buttonWidth, height}, enabled ? colors.accent : colors.controlBackground);
		draw(*item.fill, {x + width - buttonWidth * 0.5f, y}, {buttonWidth, height}, enabled ? colors.controlBackground : colors.accent);
		item.valueText->SetText(L"On");
		item.valueText->SetAnchorPoint({0.5f, 0.5f});
		item.valueText->SetPosition({x + buttonWidth * 0.5f, y});
		item.valueText->SetColor(enabled ? colors.controlText : colors.inactiveControlText);
		item.offText->SetAnchorPoint({0.5f, 0.5f});
		item.offText->SetPosition({x + width - buttonWidth * 0.5f, y});
		item.offText->SetColor(enabled ? colors.inactiveControlText : colors.controlText);
		item.offText->SetScale(scale);
		item.offText->Update();
		item.offText->Draw();
	} else {
		// 設定された枠色を残し、内側を音量に比例して塗りつぶす。
		const float value = std::clamp(item.sliderValue(), 0.0f, 1.0f);
		const float border = std::min(3.0f, height * 0.2f);
		const float innerWidth = width - border * 2.0f;
		draw(*item.track, {x + width * 0.5f, y}, {width, height}, colors.controlBackground);
		draw(*item.background, {x + width * 0.5f, y}, {innerWidth, height - border * 2.0f}, colors.controlEmpty);
		if (value > 0.0f)
			draw(*item.fill, {x + border + innerWidth * value * 0.5f, y}, {innerWidth * value, height - border * 2.0f}, colors.accent);
		item.valueText->SetText(std::to_wstring(static_cast<int>(std::round(value * 100.0f))));
		item.valueText->SetAnchorPoint({0.0f, 0.5f});
		item.valueText->SetPosition({x + width + 8.0f, y});
		item.valueText->SetColor(colors.controlText);
	}

	item.valueText->SetScale(scale);
	item.valueText->Update();
	item.valueText->Draw();
}
void MenuList::BindLayout(const std::string& key) {
	layoutKey_ = key;
	MenuLayoutParameters defaults;
	defaults.rowPanels = key != "title";

	if (key == "title") {
		defaults.colors.text = normalColor_;
		defaults.colors.selectedText = selectColor_;
	}

	defaults.position = startPos_;
	defaults.rowSpacing = offsetY_;
	defaults.textScale = textScale_;
	defaults.visibleCount = maxVisibleCount_ > 0 ? maxVisibleCount_ : std::max(1, static_cast<int>(items_.size()));
	defaults.selectorSize = selectorBgSize_;
	defaults.selectorOffset = selectorOffset_;
	defaults.controlOffsetX = checkIconOffsetX_;
	defaults.scrollBarOffsetX = scrollBarOffsetX_;

	for (const auto& item : items_) {
		if (item.toggleValue) {
			defaults.controlSize = {316.0f, 28.0f};
			break;
		}
		if (item.sliderValue) {
			defaults.controlSize = {300.0f, 28.0f};
			break;
		}
	}

	defaults.items.resize(items_.size());
	layout_ = &MenuLayoutStore::Get().Register(key, defaults);
	ApplyLayout();
}

void MenuList::ApplyLayout() {
	if (!layout_)
		return;
	if (layout_->items.size() < items_.size())
		layout_->items.resize(items_.size());
	startPos_ = layout_->position;
	offsetY_ = layout_->rowSpacing;
	textScale_ = layout_->textScale;
	maxVisibleCount_ = layout_->visibleCount;
	selectorBgSize_ = layout_->selectorSize;
	selectorOffset_ = layout_->selectorOffset;
	checkIconOffsetX_ = layout_->controlOffsetX;
	controlSize_ = layout_->controlSize;
	scrollBarOffsetX_ = layout_->scrollBarOffsetX;
	normalColor_ = layout_->colors.text;
	selectColor_ = layout_->colors.selectedText;
	selectorBg_->SetColor(layout_->colors.selector);
	scrollTrack_->SetColor(layout_->colors.scrollTrack);
	scrollThumb_->SetColor(layout_->colors.scrollThumb);
}

Vector2 MenuList::GetRowPosition(int index) const {
	const auto offset = layout_ ? layout_->items[index].offset : Vector2{};
	return {startPos_.x + offset.x, GetRowY(index) + offset.y};
}

void MenuList::Update(DirectInput* input, GamePad* gamePad, float deltaTime) {
	ApplyLayout();
	int index = static_cast<int>(items_.size());
	if (index == 0)
		return;

	const int indexBeforeInput = currentIndex_;
	const bool keyboard = ctx_->menuKeyboardAvailable;
	bool upHeld = keyboard && (input->KeyDown(DIK_W) || input->KeyDown(DIK_UP));
	bool downHeld = keyboard && (input->KeyDown(DIK_S) || input->KeyDown(DIK_DOWN));
	bool leftHeld = keyboard && (input->KeyDown(DIK_A) || input->KeyDown(DIK_LEFT));
	bool rightHeld = keyboard && (input->KeyDown(DIK_D) || input->KeyDown(DIK_RIGHT));
	bool decide = keyboard && (input->KeyTriggered(DIK_SPACE) || input->KeyTriggered(DIK_RETURN));

	if (gamePad && gamePad->GetState().connected) {
		const auto& pad = gamePad->GetState();

		// 倒し始めと戻すときのしきい値を変え、境界付近の細かな揺れを無視する。
		const auto stickDirection = [](float axis, int previous) {
			if (axis < -0.65f || (previous < 0 && axis < -0.3f))
				return -1;
			if (axis > 0.65f || (previous > 0 && axis > 0.3f))
				return 1;
			return 0;
		};

		verticalStickDirection_ = stickDirection(pad.axes.ly, verticalStickDirection_);
		horizontalStickDirection_ = stickDirection(pad.axes.lx, horizontalStickDirection_);
		upHeld = upHeld || pad.buttons.dpadUp || verticalStickDirection_ < 0;
		downHeld = downHeld || pad.buttons.dpadDown || verticalStickDirection_ > 0;
		leftHeld = leftHeld || pad.buttons.dpadLeft || horizontalStickDirection_ < 0;
		rightHeld = rightHeld || pad.buttons.dpadRight || horizontalStickDirection_ > 0;
		decide = decide || pad.buttonsPressed.a;
	} else {
		verticalStickDirection_ = horizontalStickDirection_ = 0;
	}

	const float delay = layout_ ? layout_->repeatDelay : 0.35f;
	const int vertical = static_cast<int>(downHeld) - static_cast<int>(upHeld);
	const int horizontal = static_cast<int>(rightHeld) - static_cast<int>(leftHeld);
	int navigation = navigationRepeat_.Update(vertical, deltaTime, delay, layout_ ? layout_->navigationRepeatInterval : 0.09f);
	const int adjustment = adjustmentRepeat_.Update(horizontal, deltaTime, delay, layout_ ? layout_->adjustmentRepeatInterval : 0.04f);

	if (ctx_->menuPointerAvailable) {
		if (input->GetMouseWheel() > 0.0f)
			--navigation;
		if (input->GetMouseWheel() < 0.0f)
			++navigation;
	}

	if (!input->MouseButtonDown(0)) {
		draggingSlider_ = -1;
	}

	if (!isDecided_ && ctx_->menuPointerAvailable) {
		const auto mouse = ctx_->menuPointerPosition;
		if (input->MouseButtonTriggered(0)) {
			for (int i = 0; i < index; ++i) {
				if (!IsRowVisible(i))
					continue;
				const auto pos = GetRowPosition(i);
				const float width = std::max(controlSize_.x, controlSize_.y);
				const float rightEdge = pos.x + std::max(checkIconOffsetX_ + width + 90.0f, selectorBgSize_.x);
				if (mouse.x >= pos.x && mouse.x <= rightEdge && mouse.y >= pos.y && mouse.y <= pos.y + offsetY_) {
					currentIndex_ = i;
					const float controlX = pos.x + checkIconOffsetX_;
					if (items_[i].sliderValue) {
						if (mouse.x >= controlX - 4.0f && mouse.x <= controlX + width + 4.0f)
							draggingSlider_ = i;
					} else if (items_[i].toggleValue && mouse.x >= controlX && mouse.x <= controlX + width) {
						// ボタンを直接クリックした場合は、反転ではなく選んだ状態に設定する。
						const float buttonWidth = (width - std::min(8.0f, width * 0.1f)) * 0.5f;
						if (mouse.x <= controlX + buttonWidth || mouse.x >= controlX + width - buttonWidth) {
							*items_[i].toggleValue = mouse.x < controlX + width * 0.5f;
							TinyEngine::AudioManager::Get().PlaySE(GameAudio::SE::Decide, 0.5f);
						}
					} else
						decide = true;
					break;
				}
			}
		}

		if (draggingSlider_ >= 0 && input->MouseButtonDown(0)) {
			auto& item = items_[draggingSlider_];
			const float start = GetRowPosition(draggingSlider_).x + checkIconOffsetX_;
			const float value = std::clamp((mouse.x - start) / std::max(controlSize_.x, controlSize_.y), 0.0f, 1.0f);
			item.setSliderValue(std::round(value * 100.0f) / 100.0f);
		}
	}

	if (!isDecided_ && navigation != 0) {
		currentIndex_ = ((currentIndex_ + navigation) % index + index) % index;
		TinyEngine::AudioManager::Get().PlaySE(GameAudio::SE::Select, 0.5f);
	}

	if (!isDecided_ && items_[currentIndex_].onAdjust && adjustment != 0) {
		auto& item = items_[currentIndex_];
		const float before = item.sliderValue ? item.sliderValue() : (item.toggleValue ? static_cast<float>(*item.toggleValue) : -1.0f);
		item.onAdjust(adjustment);
		const float after = item.sliderValue ? item.sliderValue() : (item.toggleValue ? static_cast<float>(*item.toggleValue) : -2.0f);
		// 音量が0や100に達した後は、選択音を鳴らし続けない。
		if (before != after)
			TinyEngine::AudioManager::Get().PlaySE(GameAudio::SE::Select, 0.5f);
	}

	// 決定処理
	if (decide && !isDecided_) {
		items_[currentIndex_].onSelect();
		TinyEngine::AudioManager::Get().PlaySE(GameAudio::SE::Decide, 0.5f);

		if (items_[currentIndex_].toggleValue || items_[currentIndex_].keepActive) {
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
	if (currentIndex_ != indexBeforeInput) {
		selectorBgAnimationTime_ = 0.0f;
	}
	selectorBgAnimationTime_ = std::min(selectorBgAnimationTime_ + deltaTime, kSelectorBgAnimationDuration);

	// 高さだけを中央に向かって縮め、元の高さに戻す
	float heightScale = 1.0f;
	if (selectorBgAnimationTime_ < kSelectorBgAnimationDuration) {
		const float progress = selectorBgAnimationTime_ / kSelectorBgAnimationDuration;
		heightScale -= (1.0f - kSelectorBgMinHeightScale) * std::sin(progress * std::numbers::pi_v<float>);
	}

	selectorBg_->SetSize({selectorBgSize_.x, selectorBgSize_.y * heightScale});
	const auto rowPosition = GetRowPosition(currentIndex_);
	selectorBg_->SetPosition({rowPosition.x + selectorOffset_.x, rowPosition.y + selectorOffset_.y});
	selectorBg_->Update();

	// 決定ボタン入力時エフェクト更新
	decideEffect_->Update(deltaTime);

	// 選択中メニューの座標をエフェクトに適用
	decideEffect_->SetPos(rowPosition);
}

void MenuList::Draw() {
	ApplyLayout();

	// 背景
	if (!layout_ || !layout_->rowPanels) {
		selectorBg_->Draw();
	}

	// 決定時エフェクト
	decideEffect_->Draw();

	for (int i = 0; i < items_.size(); i++) {
		// 表示範囲外は描画しない
		if (!IsRowVisible(i)) {
			continue;
		}

		auto& item = items_[i];
		auto& text = item.text;
		if (item.dynamicLabel) {
			const auto label = item.dynamicLabel();
			if (label != item.label) {
				item.label = label;
				text->SetText(label);
			}
		}

		// 背景を先に描き、項目名と設定部品をその上に表示する。
		const bool hasControl = item.toggleValue || item.sliderValue;
		const bool hasPanel = hasControl || (layout_ && layout_->rowPanels);
		if (hasPanel)
			DrawRowBackground(item, i);
		if (hasControl)
			DrawControl(item, i);
		text->SetColor(i == currentIndex_ ? selectColor_ : normalColor_);

		// 左上基準の座標にテキストサイズの半分を足す
		Vector2 size = text->GetTextSize();
		const auto position = GetRowPosition(i);
		const float scale = textScale_ * (layout_ ? layout_->items[i].textScale : 1.0f);
		text->SetScale(scale);
		text->SetPosition({position.x + size.x * scale * 0.5f, position.y + (hasPanel ? offsetY_ * 0.5f : size.y * scale * 0.5f)});

		// 揺れ
		float angle = 0.0f;
		if (!hasPanel && item.wiggleTimer < wiggleDuration_) {
			float t = item.wiggleTimer / wiggleDuration_;
			angle = wiggleAngle_ * std::sin(t * wiggleCycles_ * 2.0f * std::numbers::pi_v<float>) * (1.0f - t);
		}

		text->SetRotation(angle);
		text->Update();
		text->Draw();
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
