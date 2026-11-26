#pragma once
#include "AudioManager.h"

namespace GameAudio {
namespace BGM {
inline const std::string Title = "Resources/Sounds/BGM/TitleScene.mp3";
inline const std::string Game = "Resources/Sounds/BGM/GameScene.mp3";
inline const std::string Result = "Resources/Sounds/BGM/ResultScene.mp3";
}

namespace SE {
inline const std::string Cymbal = "Resources/Sounds/SE/Cymbal.mp3";
inline const std::string Decide = "Resources/Sounds/SE/Decide.mp3";
inline const std::string Error = "Resources/Sounds/SE/Error.mp3";
inline const std::string GlassBreak = "Resources/Sounds/SE/GlassBreak.mp3";
inline const std::string Heal = "Resources/Sounds/SE/Heal.mp3";
inline const std::string Piyo = "Resources/Sounds/SE/PiyoPiyo.mp3";
inline const std::string Select = "Resources/Sounds/SE/Select.mp3";
inline const std::string Shot = "Resources/Sounds/SE/Shot.mp3";
inline const std::string Snare = "Resources/Sounds/SE/Snare.mp3";
}

// 一覧にあるSEを起動時に読み込み、最初の再生からキャッシュを使えるようにする
inline void PreloadSE() {
	auto& audio = TinyEngine::AudioManager::Get();
	for (const auto* sound : {&SE::Cymbal, &SE::Decide, &SE::Error, &SE::GlassBreak, &SE::Heal, &SE::Piyo, &SE::Select, &SE::Shot, &SE::Snare}) {
		audio.Preload(*sound);
	}
}
} // namespace GameAudio
