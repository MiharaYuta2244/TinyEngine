#include "SettingsStorage.h"
#include "AudioManager.h"
#include "InputBindings.h"
#include <cmath>
#include <fstream>
#include <sstream>

namespace SettingsStorage {

std::filesystem::path Path() {
	// インストール先への書き込みを避け、ユーザーごとの設定領域を使用する
	wchar_t directory[32768]{};
	const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", directory, 32768);
	if (length == 0 || length >= 32768)
		return L"CG2-settings.txt";

	return std::filesystem::path(directory) / L"CG2" / L"settings.txt";
}

std::string Serialize(const PostEffectSettings& pfx) {
	std::ostringstream out;

	// 形式の識別子とバージョンを先頭に置き、復元時に互換性を確認する
	out << "CG2_SETTINGS 1\n";
	for (int code : InputBindings::Get().keyboard)
		out << code << ' ';
	out << '\n';
	for (int code : InputBindings::Get().pad)
		out << code << ' ';
	out << '\n' << TinyEngine::AudioManager::GetBGMVolume() << ' ' << TinyEngine::AudioManager::GetSEVolume() << '\n';
	out << pfx.vignetteEnabled << ' ' << pfx.outlineEnabled << ' ' << pfx.radialBlurEnabled << ' ' << pfx.glitchEnabled << ' ' << pfx.scanlineEnabled << ' ' << pfx.barrelDistortionEnabled << ' '
	    << pfx.blurEnabled << ' ' << pfx.deathEffectEnabled << '\n';

	return out.str();
}

bool Save(const PostEffectSettings& pfx) {
	const auto path = Path();
	std::error_code error;

	if (path.has_parent_path())
		std::filesystem::create_directories(path.parent_path(), error);

	if (error)
		return false;

	// 書き込み途中に失敗しても、既存の設定ファイルを残せるようにする
	auto temporary = path;
	temporary += L".tmp";
	std::ofstream out(temporary, std::ios::trunc);
	out << Serialize(pfx);
	out.close();

	if (!out)
		return false;

	// 書き込みとクローズが成功してから、保存先を一時ファイルで置き換える
	return MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
}

void Load(PostEffectSettings& pfx) {
	// 読み取り中は一時変数を使用し、途中の失敗による部分反映を防ぐ
	std::ifstream in(Path());
	std::string magic;
	int version = 0;
	std::array<int, InputBindings::kActionCount> keyboard{}, pad{};
	float bgm = 1.0f, se = 1.0f;
	PostEffectSettings effects;

	// 形式、入力コード、割り当ての重複、音量の範囲を順に検証する
	if (!(in >> magic >> version) || magic != "CG2_SETTINGS" || version != 1)
		return;

	for (int& code : keyboard)
		if (!(in >> code) || !InputBindings::ValidKeyboard(code))
			return;

	for (int& code : pad)
		if (!(in >> code) || code < 0 || code >= static_cast<int>(PadBinding::Count))
			return;

	for (int i = 0; i < InputBindings::kActionCount; ++i) {
		for (int j = 0; j < i; ++j)
			if (keyboard[i] == keyboard[j] || pad[i] == pad[j])
				return;
	}

	if (!(in >> bgm >> se) || !std::isfinite(bgm) || !std::isfinite(se) || bgm < 0 || bgm > 1 || se < 0 || se > 1)
		return;

	if (!(in >> effects.vignetteEnabled >> effects.outlineEnabled >> effects.radialBlurEnabled >> effects.glitchEnabled >> effects.scanlineEnabled >> effects.barrelDistortionEnabled >>
	      effects.blurEnabled >> effects.deathEffectEnabled))
		return;

	// すべての項目を読み取れた場合に限り、ゲーム内の共有設定へ反映する
	InputBindings::Get().keyboard = keyboard;
	InputBindings::Get().pad = pad;
	pfx = effects;
	TinyEngine::AudioManager::SetBGMVolume(bgm);
	TinyEngine::AudioManager::SetSEVolume(se);
}
} // namespace SettingsStorage
