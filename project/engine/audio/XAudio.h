#pragma once
#include <array>
#include <assert.h>
#include <fstream>
#include <vector>
#include <wrl.h>
#include <xaudio2.h>

#pragma comment(lib, "xaudio2.lib")

// チャンクヘッダ
struct ChunkHeader {
	char id[4];   // チャンク毎のID
	int32_t size; // チャンクサイズ
};

// RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk; // "RIFF"
	char type[4];      // "WAVE"
};

// FMTチャンク
struct FormatChunk {
	ChunkHeader chunk;          // "fmt"
	std::array<BYTE, 40> fmt{}; // 波形フォーマット
};

// 音声データ
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファ
	std::vector<BYTE> buffer;
};

class XAudio {
public:
	~XAudio();
	void Initialize();
	void SoundsAllLoad();
	void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData);
	Microsoft::WRL::ComPtr<IXAudio2> GetXAudio2() { return xAudio2_; }
	SoundData GetSound() { return soundData_; }

private:
	void SoundLoadFile(const std::string& filename);
	void SoundUnLoad(SoundData* soundData);

private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	SoundData soundData_;
};
