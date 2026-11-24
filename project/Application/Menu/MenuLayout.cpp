#include "MenuLayout.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#ifdef USE_IMGUI
#include "imgui.h"
#endif

namespace {
using Json = nlohmann::json;

#ifdef USE_IMGUI
const char* DisplayName(const std::string& key) {
	if (key == "title")
		return "タイトル";
	if (key == "pause")
		return "ポーズ";
	if (key == "option.root")
		return "オプション：項目選択";
	if (key == "option.effects")
		return "オプション：ポストエフェクト";
	if (key == "option.input")
		return "オプション：入力機器選択";
	if (key == "option.audio")
		return "オプション：BGM・SE";
	if (key == "option.keyboard")
		return "オプション：キーボード・マウス";
	if (key == "option.gamepad")
		return "オプション：ゲームパッド";
	return key.c_str();
}
#endif

float ReadNumber(const Json& data, const char* key, float fallback, float minimum, float maximum) {
	if (!data.contains(key))
		return fallback;
	const float value = data.at(key).get<float>();
	if (!std::isfinite(value))
		throw std::runtime_error("数値が有限ではありません");
	return std::clamp(value, minimum, maximum);
}

Vector2 ReadVector(const Json& data, const char* key, Vector2 fallback, float minimum, float maximum) {
	if (!data.contains(key))
		return fallback;
	const auto& value = data.at(key);
	if (!value.is_array() || value.size() != 2)
		throw std::runtime_error("座標の要素数が不正です");
	const float x = value.at(0).get<float>(), y = value.at(1).get<float>();
	if (!std::isfinite(x) || !std::isfinite(y))
		throw std::runtime_error("座標が有限ではありません");
	return {std::clamp(x, minimum, maximum), std::clamp(y, minimum, maximum)};
}

Vector4 ReadColor(const Json& data, const char* key, Vector4 fallback) {
	if (!data.contains(key))
		return fallback;
	const auto& color = data.at(key);
	if (!color.is_array() || color.size() != 4)
		throw std::runtime_error("色の要素数が不正です");
	Vector4 result{};
	float* components[] = {&result.x, &result.y, &result.z, &result.w};
	for (int i = 0; i < 4; ++i) {
		const float value = color.at(i).get<float>();
		if (!std::isfinite(value))
			throw std::runtime_error("色が有限ではありません");
		*components[i] = std::clamp(value, 0.0f, 1.0f);
	}
	return result;
}

MenuLayoutParameters Decode(const Json& data, MenuLayoutParameters value) {
	if (!data.is_object())
		throw std::runtime_error("メニューの形式が不正です");
	if (data.contains("rowPanels"))
		value.rowPanels = data.at("rowPanels").get<bool>();
	if (data.contains("colors")) {
		const auto& colors = data.at("colors");
		if (!colors.is_object())
			throw std::runtime_error("配色の形式が不正です");
		value.colors.rowBackground = ReadColor(colors, "rowBackground", value.colors.rowBackground);
		value.colors.selectedRowBackground = ReadColor(colors, "selectedRowBackground", value.colors.selectedRowBackground);
		value.colors.text = ReadColor(colors, "text", value.colors.text);
		value.colors.selectedText = ReadColor(colors, "selectedText", value.colors.selectedText);
		value.colors.accent = ReadColor(colors, "accent", value.colors.accent);
		value.colors.controlBackground = ReadColor(colors, "controlBackground", value.colors.controlBackground);
		value.colors.controlEmpty = ReadColor(colors, "controlEmpty", value.colors.controlEmpty);
		value.colors.controlText = ReadColor(colors, "controlText", value.colors.controlText);
		value.colors.inactiveControlText = ReadColor(colors, "inactiveControlText", value.colors.inactiveControlText);
		value.colors.heading = ReadColor(colors, "heading", value.colors.heading);
		value.colors.help = ReadColor(colors, "help", value.colors.help);
		value.colors.selector = ReadColor(colors, "selector", value.colors.selector);
		value.colors.scrollTrack = ReadColor(colors, "scrollTrack", value.colors.scrollTrack);
		value.colors.scrollThumb = ReadColor(colors, "scrollThumb", value.colors.scrollThumb);
	}

	value.position = ReadVector(data, "position", value.position, -3000.0f, 3000.0f);
	value.rowSpacing = ReadNumber(data, "rowSpacing", value.rowSpacing, 10.0f, 200.0f);
	value.textScale = ReadNumber(data, "textScale", value.textScale, 0.2f, 3.0f);
	if (data.contains("visibleCount"))
		value.visibleCount = std::clamp(data.at("visibleCount").get<int>(), 1, 20);
	value.selectorSize = ReadVector(data, "selectorSize", value.selectorSize, 1.0f, 3000.0f);
	value.selectorOffset = ReadVector(data, "selectorOffset", value.selectorOffset, -3000.0f, 3000.0f);
	value.controlOffsetX = ReadNumber(data, "controlOffsetX", value.controlOffsetX, -2000.0f, 2000.0f);
	value.controlSize = ReadVector(data, "controlSize", value.controlSize, 8.0f, 600.0f);
	value.scrollBarOffsetX = ReadNumber(data, "scrollBarOffsetX", value.scrollBarOffsetX, -2000.0f, 2000.0f);
	value.headingPosition = ReadVector(data, "headingPosition", value.headingPosition, -3000.0f, 3000.0f);
	value.headingScale = ReadNumber(data, "headingScale", value.headingScale, 0.2f, 3.0f);
	value.helpPosition = ReadVector(data, "helpPosition", value.helpPosition, -3000.0f, 3000.0f);
	value.helpScale = ReadNumber(data, "helpScale", value.helpScale, 0.2f, 3.0f);
	value.repeatDelay = ReadNumber(data, "repeatDelay", value.repeatDelay, 0.05f, 2.0f);
	value.navigationRepeatInterval = ReadNumber(data, "navigationRepeatInterval", value.navigationRepeatInterval, 0.02f, 1.0f);
	value.adjustmentRepeatInterval = ReadNumber(data, "adjustmentRepeatInterval", value.adjustmentRepeatInterval, 0.01f, 1.0f);

	if (data.contains("items")) {
		const auto& items = data.at("items");
		if (!items.is_array() || items.size() > 100)
			throw std::runtime_error("項目の形式が不正です");
		value.items.clear();
		for (const auto& item : items) {
			if (!item.is_object())
				throw std::runtime_error("項目の形式が不正です");
			value.items.push_back({ReadVector(item, "offset", {}, -3000.0f, 3000.0f), ReadNumber(item, "textScale", 1.0f, 0.2f, 3.0f)});
		}
	}
	return value;
}

Json Encode(const MenuLayoutParameters& value) {
	const auto vector = [](Vector2 v) { return Json::array({v.x, v.y}); };
	Json result = {
	    {"position",         vector(value.position)       },
	    {"rowSpacing",       value.rowSpacing             },
	    {"textScale",        value.textScale              },
	    {"visibleCount",     value.visibleCount           },
	    {"selectorSize",     vector(value.selectorSize)   },
	    {"selectorOffset",   vector(value.selectorOffset) },
	    {"controlOffsetX",   value.controlOffsetX         },
	    {"controlSize",      vector(value.controlSize)    },
	    {"scrollBarOffsetX", value.scrollBarOffsetX       },
	    {"headingPosition",  vector(value.headingPosition)},
	    {"headingScale",     value.headingScale           },
	    {"helpPosition",     vector(value.helpPosition)   },
	    {"helpScale",        value.helpScale              }
    };

	const auto color = [](Vector4 v) { return Json::array({v.x, v.y, v.z, v.w}); };

	result["rowPanels"] = value.rowPanels;
	result["colors"] = {
	    {"rowBackground",         color(value.colors.rowBackground)        },
	    {"selectedRowBackground", color(value.colors.selectedRowBackground)},
	    {"text",	              color(value.colors.text)                 },
	    {"selectedText",          color(value.colors.selectedText)         },
	    {"accent",                color(value.colors.accent)               },
	    {"controlBackground",     color(value.colors.controlBackground)    },
	    {"controlEmpty",          color(value.colors.controlEmpty)         },
	    {"controlText",           color(value.colors.controlText)          },
	    {"inactiveControlText",   color(value.colors.inactiveControlText)  },
	    {"heading",               color(value.colors.heading)              },
	    {"help",	              color(value.colors.help)                 },
	    {"selector",              color(value.colors.selector)             },
	    {"scrollTrack",           color(value.colors.scrollTrack)          },
	    {"scrollThumb",           color(value.colors.scrollThumb)          }
    };

	result["items"] = Json::array();
	result["repeatDelay"] = value.repeatDelay;
	result["navigationRepeatInterval"] = value.navigationRepeatInterval;
	result["adjustmentRepeatInterval"] = value.adjustmentRepeatInterval;

	for (const auto& item : value.items) {
		result["items"].push_back({
		    {"offset",    vector(item.offset)},
            {"textScale", item.textScale     }
        });
	}
	return result;
}
} // namespace

MenuLayoutStore& MenuLayoutStore::Get() {
	static MenuLayoutStore store;
	return store;
}

const char* MenuLayoutStore::FilePath() { return "Resources/Json/UI/MenuLayout.json"; }

MenuLayoutParameters& MenuLayoutStore::Register(const std::string& key, const MenuLayoutParameters& defaults) {
	defaults_.try_emplace(key, defaults);
	auto [entry, inserted] = layouts_.try_emplace(key, defaults);
	// 未生成の画面の設定も、初めてその画面を開く際に保存値から復元する
	if (inserted && !loadedJson_.empty()) {
		const auto data = Json::parse(loadedJson_);
		if (data.at("menus").contains(key))
			entry->second = Decode(data.at("menus").at(key), defaults);
	}
	return entry->second;
}

MenuLayoutParameters& MenuLayoutStore::Layout(const std::string& key) { return layouts_.at(key); }

bool MenuLayoutStore::Load(const std::string& path) {
	try {
		std::ifstream input(path);
		if (!input)
			return false;
		Json data;
		input >> data;
		if (data.at("version").get<int>() != 1 || !data.at("menus").is_object())
			return false;
		// 全階層を検証してから反映し、壊れたJSONで一部だけ更新されることを防ぐ
		for (const auto& entry : data.at("menus").items())
			Decode(entry.value(), {});
		auto updated = layouts_;
		for (auto& [key, value] : updated) {
			value = defaults_.at(key);
			if (data.at("menus").contains(key))
				value = Decode(data.at("menus").at(key), value);
		}
		for (const auto& [key, value] : updated)
			layouts_.at(key) = value;
		loadedJson_ = data.dump();
		return true;
	} catch (const std::exception&) {
		return false;
	}
}

bool MenuLayoutStore::Save(const std::string& path) const {
	try {
		// まだ開いていない画面の設定も消さずに残す
		Json data = loadedJson_.empty() ? Json{{"version", 1}, {"menus", Json::object()}} : Json::parse(loadedJson_);
		for (const auto& [key, value] : layouts_)
			data["menus"][key] = Encode(value);
		const std::filesystem::path destination(path);
		if (destination.has_parent_path())
			std::filesystem::create_directories(destination.parent_path());
		auto temporary = destination;
		temporary += L".tmp";
		std::ofstream output(temporary);
		output << data.dump(4) << '\n';
		output.close();
		if (!output)
			return false;
		return MoveFileExW(temporary.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
	} catch (const std::exception&) {
		return false;
	}
}

std::string MenuLayoutStore::DrawImGui() {
#ifdef USE_IMGUI
	if (ImGui::Begin("メニューレイアウト")) {
		ImGui::TextUnformatted(FilePath());
		if (ImGui::BeginCombo("編集する画面", DisplayName(selectedKey_))) {
			for (const auto& [key, value] : layouts_) {
				(void)value;
				if (ImGui::Selectable(DisplayName(key), key == selectedKey_))
					selectedKey_ = key;
			}
			ImGui::EndCombo();
		}
		ImGui::Checkbox("編集する階層をプレビュー", &preview_);
		if (layouts_.contains(selectedKey_)) {
			auto& layout = layouts_.at(selectedKey_);
			if (ImGui::TreeNode("配色（透明度も変更できます）")) {
				ImGui::ColorEdit4("行の背景", &layout.colors.rowBackground.x);
				ImGui::ColorEdit4("選択中の行の背景", &layout.colors.selectedRowBackground.x);
				ImGui::ColorEdit4("項目の文字", &layout.colors.text.x);
				ImGui::ColorEdit4("選択中の文字", &layout.colors.selectedText.x);
				ImGui::ColorEdit4("選択線・有効ボタン・音量バー", &layout.colors.accent.x);
				ImGui::ColorEdit4("無効ボタン・音量の外枠", &layout.colors.controlBackground.x);
				ImGui::ColorEdit4("音量バーの空き領域", &layout.colors.controlEmpty.x);
				ImGui::ColorEdit4("有効ボタン・音量数値の文字", &layout.colors.controlText.x);
				ImGui::ColorEdit4("無効ボタンの文字", &layout.colors.inactiveControlText.x);
				ImGui::ColorEdit4("見出しの文字", &layout.colors.heading.x);
				ImGui::ColorEdit4("操作説明の文字", &layout.colors.help.x);
				ImGui::ColorEdit4("タイトルの選択背景", &layout.colors.selector.x);
				ImGui::ColorEdit4("スクロールバーの背景", &layout.colors.scrollTrack.x);
				ImGui::ColorEdit4("スクロールバーのつまみ", &layout.colors.scrollThumb.x);
				if (selectedKey_ != "title" && ImGui::Button("この配色を他の設定階層にも反映")) {
					// 最上位のタイトルメニューは変更対象に含めない。
					for (auto& [key, target] : layouts_)
						if (key != "title")
							target.colors = layout.colors;
				}
				ImGui::TreePop();
			}
			ImGui::DragFloat2("基準位置", &layout.position.x, 1.0f, -3000.0f, 3000.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat("行間", &layout.rowSpacing, 1.0f, 10.0f, 200.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat("文字倍率", &layout.textScale, 0.01f, 0.2f, 3.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::SliderInt("表示行数", &layout.visibleCount, 1, 20, "%d", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat2("選択枠サイズ", &layout.selectorSize.x, 1.0f, 1.0f, 3000.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat2("選択枠の相対位置", &layout.selectorOffset.x, 1.0f, -3000.0f, 3000.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat("部品の横位置", &layout.controlOffsetX, 1.0f, -2000.0f, 2000.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat2("ON/OFFボタン・音量バーのサイズ", &layout.controlSize.x, 1.0f, 8.0f, 600.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat("長押し開始までの秒数", &layout.repeatDelay, 0.01f, 0.05f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat("項目移動の間隔（秒）", &layout.navigationRepeatInterval, 0.01f, 0.02f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat("音量変更の間隔（秒）", &layout.adjustmentRepeatInterval, 0.01f, 0.01f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat("スクロールバーの横位置", &layout.scrollBarOffsetX, 1.0f, -2000.0f, 2000.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat2("見出し位置", &layout.headingPosition.x, 1.0f, -3000.0f, 3000.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat("見出し倍率", &layout.headingScale, 0.01f, 0.2f, 3.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat2("操作説明位置", &layout.helpPosition.x, 1.0f, -3000.0f, 3000.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::DragFloat("操作説明倍率", &layout.helpScale, 0.01f, 0.2f, 3.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			if (ImGui::TreeNode("各項目の調整（上から順番）")) {
				for (int i = 0; i < static_cast<int>(layout.items.size()); ++i) {
					ImGui::PushID(i);
					ImGui::Text("項目 %d", i + 1);
					ImGui::DragFloat2("相対位置", &layout.items[i].offset.x, 1.0f, -3000.0f, 3000.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
					ImGui::DragFloat("個別文字倍率", &layout.items[i].textScale, 0.01f, 0.2f, 3.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			if (ImGui::Button("この画面を初期値に戻す"))
				layout = defaults_.at(selectedKey_);
		}
		if (ImGui::Button("JSONに保存"))
			status_ = Save() ? "保存しました" : "保存に失敗しました";
		ImGui::SameLine();
		if (ImGui::Button("JSONを再読込"))
			status_ = Load() ? "読み込みました" : "読み込みに失敗しました。現在の値を維持します";
		ImGui::TextWrapped("%s", status_.c_str());
	}
	ImGui::End();
#endif
	return preview_ ? selectedKey_ : std::string{};
}
