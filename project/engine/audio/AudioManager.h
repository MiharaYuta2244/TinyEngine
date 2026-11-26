#pragma once
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <wrl.h>
#include <xaudio2.h>

#pragma comment(lib, "xaudio2.lib")

struct SoundData {
	WAVEFORMATEXTENSIBLE wfex{};
	std::vector<BYTE> buffer;
};

class Game;

/// <summary>
/// 音声管理クラス
/// </summary>
namespace TinyEngine {
class AudioManager {
public:
	// ゲーム全体で共有するインスタンスを取得する。音声デバイスは起動時に別途初期化
	static AudioManager& Get();

	// Gameのメインループからだけ音声システムの寿命を管理する
	class Lifecycle {
		friend class ::Game;

		// 起動時に共有の音声システムを初期化する
		static void Initialize() { AudioManager::Get().Initialize(); }
		// 毎フレーム、再生が終わったSEを整理する
		static void Update() { AudioManager::Get().Update(); }
		// ゲーム終了時に再生を止め、音声リソースを解放する
		static void Finalize() { AudioManager::Get().Finalize(); }
	};

	// 指定した音声を再生せずに読み込み、キャッシュに登録する関数
	void Preload(const std::string& filename);

	// BGMの再生
	void PlayBGM(const std::string& filename, float volume = 0.1f);

	// BGMの停止
	void StopBGM();

	// SEの再生
	void PlaySE(const std::string& filename, float volume = 0.1f);

	// BGMの全体音量を0〜1に収め、再生中のBGMにも反映する関数
	void SetBGMVolume(float volume);

	// SEの全体音量を0〜1に収め、再生中のすべてのSEにも反映する関数
	void SetSEVolume(float volume);

	// 現在のBGM全体音量Getter
	float GetBGMVolume() const { return bgmVolume_; }

	// 現在のSE全体音量Getter
	float GetSEVolume() const { return seVolume_; }

	// BGMの現在位置から64個の周波数成分を求める。停止中はすべて0を返す
	std::vector<float> GetSpectrum() const;

private:
	// 初期化関数
	void Initialize();

	// 更新関数
	void Update();

	// 終了時処理
	void Finalize();

	// コンストラクタ
	AudioManager() = default;

	// デストラクタ
	~AudioManager();

	// 共有インスタンスのコピー生成を禁止する
	AudioManager(const AudioManager&) = delete;

	// 共有インスタンスへのコピー代入を禁止する
	AudioManager& operator=(const AudioManager&) = delete;

	struct VoiceDeleter {
		// 所有しているXAudio2の再生ボイスを破棄する
		void operator()(IXAudio2SourceVoice* voice) const { voice->DestroyVoice(); }
	};

	using Voice = std::unique_ptr<IXAudio2SourceVoice, VoiceDeleter>;

private:
	// ファイルパスでキャッシュを検索し、未読込ならデコードする。失敗時はnullptrを返す
	const SoundData* GetOrLoadSound(const std::string& filename);

	// ボイスを作成し、指定の音量とループ設定で再生を開始する。失敗時は空のVoiceを返す
	Voice StartVoice(const SoundData& data, float volume, bool loop);

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_ = nullptr;
	std::unordered_map<std::string, SoundData> soundData_;

	Voice bgmVoice_;
	const SoundData* currentBgm_ = nullptr;
	float bgmBaseVolume_ = 0.1f;
	float bgmVolume_ = 1.0f;

	struct SEVoice {
		Voice voice;
		float volume;
	};

	std::list<SEVoice> seVoices_;
	float seVolume_ = 1.0f;
};
} // namespace TinyEngine
