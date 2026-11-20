#pragma once
#include "Vector2.h"
#include "Vector4.h"
#include <map>
#include <string>
#include <vector>

// 各項目の座標は、メニューの基準位置と行間から求めた位置への加算値
struct MenuItemLayout {
	Vector2 offset{};
	float textScale = 1.0f;
};

// 色はRGBAの0～1で管理し、各階層で個別に調整・保存できるようにする
struct MenuPalette {
	Vector4 rowBackground = {1.0f, 0.94f, 0.8f, 1.0f};
	Vector4 selectedRowBackground = {1.0f, 0.94f, 0.8f, 1.0f};
	Vector4 text = {0.29f, 0.0f, 0.39f, 1.0f};
	Vector4 selectedText = {0.29f, 0.0f, 0.39f, 1.0f};
	Vector4 accent = {1.0f, 0.8f, 0.18f, 1.0f};
	Vector4 controlBackground = {0.29f, 0.0f, 0.39f, 1.0f};
	Vector4 controlEmpty = {1.0f, 0.94f, 0.8f, 1.0f};
	Vector4 controlText = {0.29f, 0.0f, 0.39f, 1.0f};
	Vector4 inactiveControlText = {0.68f, 0.58f, 0.68f, 1.0f};
	Vector4 heading = {1.0f, 1.0f, 1.0f, 1.0f};
	Vector4 help = {1.0f, 1.0f, 1.0f, 1.0f};
	Vector4 selector = {1.0f, 1.0f, 1.0f, 0.2f};
	Vector4 scrollTrack = {1.0f, 1.0f, 1.0f, 0.15f};
	Vector4 scrollThumb = {1.0f, 0.8f, 0.18f, 1.0f};
};

struct MenuLayoutParameters {
	bool rowPanels = true;
	MenuPalette colors;
	Vector2 position = {400.0f, 290.0f};
	float rowSpacing = 46.0f;
	float textScale = 1.0f;
	int visibleCount = 7;
	Vector2 selectorSize = {900.0f, 42.0f};
	Vector2 selectorOffset = {350.0f, 21.0f};
	float controlOffsetX = 430.0f;
	Vector2 controlSize = {180.0f, 32.0f};
	float scrollBarOffsetX = 690.0f;
	Vector2 headingPosition = {400.0f, 230.0f};
	float headingScale = 1.0f;
	Vector2 helpPosition = {100.0f, 650.0f};
	float helpScale = 0.85f;
	float repeatDelay = 0.35f;
	float navigationRepeatInterval = 0.09f;
	float adjustmentRepeatInterval = 0.04f;
	std::vector<MenuItemLayout> items;
};

/// <summary>
/// 制作時のレイアウトを共有する。プレイヤーの音量・キー設定とは別ファイルで管理する
/// </summary>
class MenuLayoutStore {
public:
	static MenuLayoutStore& Get();
	static const char* FilePath();
	MenuLayoutParameters& Register(const std::string& key, const MenuLayoutParameters& defaults);
	MenuLayoutParameters& Layout(const std::string& key);
	bool Load(const std::string& path = FilePath());
	bool Save(const std::string& path = FilePath()) const;

	// 選択中の編集対象を返す。起動画面で対応する階層をプレビューするために使用する
	std::string DrawImGui();

private:
	std::map<std::string, MenuLayoutParameters> layouts_, defaults_;
	std::string loadedJson_;
	std::string selectedKey_ = "title";
	std::string status_;
	bool preview_ = false;
};
