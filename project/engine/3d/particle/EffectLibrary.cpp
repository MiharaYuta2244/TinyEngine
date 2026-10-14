#include "EffectLibrary.h"
#include "JsonManager.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace {
const std::string kDir = "resources/json/Effects";
std::string RelPath(const std::string& name) { return "Effects/" + name + ".json"; }
} // namespace

EffectLibrary& EffectLibrary::Get() {
	static EffectLibrary instance;
	return instance;
}

bool EffectLibrary::IsValidName(const std::string& name) {
	// ファイル名に使えない文字・空白・空文字は不可
	return !name.empty() && name.find_first_of("\\/:*?\"<>|. ") == std::string::npos;
}

void EffectLibrary::LoadAll() {
	effects_.clear();
	if (!fs::exists(kDir)) {
		return;
	}
	for (const auto& entry : fs::directory_iterator(kDir)) {
		if (!entry.is_regular_file() || entry.path().extension() != ".json") {
			continue;
		}
		std::string stem = entry.path().stem().string();
		EffectData data;
		if (JsonManager::Load(RelPath(stem), data)) {
			data.name = stem;
			effects_[stem] = std::move(data);
		}
	}
}

const EffectData* EffectLibrary::Find(const std::string& name) const {
	auto it = effects_.find(name);
	return it != effects_.end() ? &it->second : nullptr;
}

EffectData* EffectLibrary::FindMutable(const std::string& name) {
	auto it = effects_.find(name);
	return it != effects_.end() ? &it->second : nullptr;
}

std::vector<std::string> EffectLibrary::GetNames() const {
	std::vector<std::string> names;
	for (auto& [name, _] : effects_) {
		names.push_back(name);
	}
	return names;
}

EffectData* EffectLibrary::Create(const std::string& name) {
	if (!IsValidName(name) || effects_.contains(name)) {
		return nullptr;
	}
	EffectData data;
	data.name = name;
	data.emitters.emplace_back(); // デフォルトのエミッターを1つ
	return &effects_.emplace(name, std::move(data)).first->second;
}

bool EffectLibrary::Duplicate(const std::string& src, const std::string& dst) {
	const EffectData* from = Find(src);
	if (!from || !IsValidName(dst) || effects_.contains(dst)) {
		return false;
	}
	EffectData copy = *from;
	copy.name = dst;
	effects_.emplace(dst, std::move(copy));
	return true;
}

bool EffectLibrary::Rename(const std::string& from, const std::string& to) {
	auto it = effects_.find(from);
	if (it == effects_.end() || !IsValidName(to) || effects_.contains(to)) {
		return false;
	}
	EffectData data = std::move(it->second);
	data.name = to;
	effects_.erase(it);
	fs::remove(fs::path(kDir) / (from + ".json"));
	effects_.emplace(to, std::move(data));
	return Save(to);
}

bool EffectLibrary::Remove(const std::string& name) {
	if (effects_.erase(name) == 0) {
		return false;
	}
	std::error_code ec;
	fs::remove(fs::path(kDir) / (name + ".json"), ec);
	return true;
}

bool EffectLibrary::Save(const std::string& name) {
	const EffectData* data = Find(name);
	return data && JsonManager::Save(RelPath(name), *data);
}

void EffectLibrary::SaveAll() {
	for (auto& [name, _] : effects_) {
		Save(name);
	}
}