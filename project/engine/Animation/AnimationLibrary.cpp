#include "AnimationLibrary.h"

std::unordered_map<std::string, Animation>& AnimationLibrary::Cache() {
	static std::unordered_map<std::string, Animation> cache;
	return cache;
}

const Animation& AnimationLibrary::Get(const std::string& file) {
	auto& cache = Cache();

	auto it = cache.find(file);
	if (it == cache.end()) {
		// 初回のみAssimpでファイルを読む
		KeyframeAnimation loader;
		it = cache.emplace(file, loader.LoadAnimationFile(file)).first;
	}
	return it->second;
}

void AnimationLibrary::Clear() { Cache().clear(); }
