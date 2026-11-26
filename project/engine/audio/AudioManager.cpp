#include "AudioManager.h"
#include "FFT.h"
#include "Logger.h"
#include "StringUtility.h"
#include <algorithm>
#include <cstring>
#include <format>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <utility>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

using Microsoft::WRL::ComPtr;
using namespace TinyEngine;

namespace {
// Media Foundationで音声ファイルをデコードし、XAudio2に渡せるPCMデータを作る
HRESULT DecodeSound(const std::string& filename, SoundData& sound) {
	ComPtr<IMFSourceReader> reader;
	HRESULT result = MFCreateSourceReaderFromURL(StringUtility::ConvertString(filename).c_str(), nullptr, &reader);
	if (FAILED(result)) return result;

	// 最初の音声ストリームを非圧縮のPCMとして読み出す
	ComPtr<IMFMediaType> pcmType;
	result = MFCreateMediaType(&pcmType);
	if (FAILED(result)) return result;

	result = pcmType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	if (FAILED(result)) return result;

	result = pcmType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	if (FAILED(result)) return result;

	constexpr DWORD stream = MF_SOURCE_READER_FIRST_AUDIO_STREAM;
	result = reader->SetCurrentMediaType(stream, nullptr, pcmType.Get());
	if (FAILED(result)) return result;

	ComPtr<IMFMediaType> outputType;
	result = reader->GetCurrentMediaType(stream, &outputType);
	if (FAILED(result)) return result;

	// デコーダーが決定したサンプルレートやチャンネル数を再生用の書式に変換する
	WAVEFORMATEX* format = nullptr;
	UINT32 formatSize = 0;
	result = MFCreateWaveFormatExFromMFMediaType(outputType.Get(), &format, &formatSize);
	if (FAILED(result)) return result;

	const std::unique_ptr<WAVEFORMATEX, decltype(&CoTaskMemFree)> formatOwner(format, CoTaskMemFree);
	if (formatSize > sizeof(sound.wfex)) return MF_E_INVALIDMEDIATYPE;

	// PCMのWAVEFORMATEXはWAVEFORMATEXTENSIBLEより短い場合がある
	std::memcpy(&sound.wfex, formatOwner.get(), formatSize);

	// ファイルの末尾までPCMの断片を読み取り、1つの再生バッファにつなげる
	while (true) {
		ComPtr<IMFSample> sample;
		DWORD flags = 0;
		result = reader->ReadSample(stream, 0, nullptr, &flags, nullptr, &sample);
		if (FAILED(result)) return result;
		if (flags & (MF_SOURCE_READERF_ERROR | MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)) return MF_E_INVALIDMEDIATYPE;

		if (sample) {
			ComPtr<IMFMediaBuffer> buffer;
			result = sample->ConvertToContiguousBuffer(&buffer);
			if (FAILED(result)) return result;

			DWORD length = 0;
			result = buffer->GetCurrentLength(&length);
			if (FAILED(result)) return result;
			if (length > XAUDIO2_MAX_BUFFER_BYTES - sound.buffer.size()) return E_INVALIDARG;

			if (length != 0) {
				const size_t offset = sound.buffer.size();
				sound.buffer.resize(offset + length);
				BYTE* bytes = nullptr;
				result = buffer->Lock(&bytes, nullptr, nullptr);
				if (FAILED(result)) return result;

				std::memcpy(sound.buffer.data() + offset, bytes, length);
				buffer->Unlock();
			}
		}

		if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
			break;
		}
	}

	return sound.buffer.empty() ? MF_E_INVALIDMEDIATYPE : S_OK;
}
} // namespace

AudioManager& AudioManager::Get() {
	static AudioManager instance;
	return instance;
}

AudioManager::~AudioManager() { Finalize(); }

void AudioManager::Initialize() {
	if (masterVoice_) return;

	HRESULT result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (SUCCEEDED(result)) {
		result = xAudio2_->CreateMasteringVoice(&masterVoice_);
	}
	if (SUCCEEDED(result)) {
		result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
		if (SUCCEEDED(result)) return;

		// デコード機能の初期化に失敗した場合は、先に確保した出力ボイスを解放する
		masterVoice_->DestroyVoice();
		masterVoice_ = nullptr;
	}

	xAudio2_.Reset();
	Logger::Log(std::format("Audio initialization failed ({:#010x})", static_cast<unsigned long>(result)), LogLevel::Error);
}

void AudioManager::Finalize() {
	if (!masterVoice_) return;

	// 再生ボイスはキャッシュ内のPCMを参照しているため、先に破棄する
	StopBGM();
	seVoices_.clear();
	soundData_.clear();

	masterVoice_->DestroyVoice();
	masterVoice_ = nullptr;
	xAudio2_.Reset();
	MFShutdown();
}

void AudioManager::Update() {
	seVoices_.remove_if([](const SEVoice& se) {
		XAUDIO2_VOICE_STATE state{};
		se.voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);

		// リストから削除すると、Voiceのデリーターが再生ボイスを破棄する
		return state.BuffersQueued == 0;
	});
}

const SoundData* AudioManager::GetOrLoadSound(const std::string& filename) {
	if (!masterVoice_) return nullptr;
	if (const auto it = soundData_.find(filename); it != soundData_.end()) return &it->second;

	// 未読込のファイルだけをデコードし、成功したデータをキャッシュに登録する
	SoundData sound;
	const HRESULT result = DecodeSound(filename, sound);
	if (FAILED(result)) {
		Logger::Log(std::format("Audio load failed ({:#010x}): {}", static_cast<unsigned long>(result), filename), LogLevel::Error);
		return nullptr;
	}

	return &soundData_.try_emplace(filename, std::move(sound)).first->second;
}

void AudioManager::Preload(const std::string& filename) { GetOrLoadSound(filename); }

AudioManager::Voice AudioManager::StartVoice(const SoundData& data, float volume, bool loop) {
	IXAudio2SourceVoice* source = nullptr;
	HRESULT result = xAudio2_->CreateSourceVoice(&source, &data.wfex.Format);
	Voice voice(source);
	if (SUCCEEDED(result)) {
		XAUDIO2_BUFFER buffer{};

		// PCMをコピーせず、キャッシュ内のバッファを再生ボイスから参照する
		buffer.pAudioData = data.buffer.data();
		buffer.AudioBytes = static_cast<UINT32>(data.buffer.size());
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

		result = voice->SubmitSourceBuffer(&buffer);
		if (SUCCEEDED(result)) {
			result = voice->SetVolume(volume);
		}
		if (SUCCEEDED(result)) {
			result = voice->Start();
		}
	}

	if (FAILED(result)) {
		Logger::Log(std::format("Audio playback failed ({:#010x})", static_cast<unsigned long>(result)), LogLevel::Error);
		return {};
	}
	return voice;
}

void AudioManager::PlayBGM(const std::string& filename, float volume) {
	const SoundData* sound = GetOrLoadSound(filename);
	if (!sound) return;

	Voice voice = StartVoice(*sound, volume * bgmVolume_, true);
	if (!voice) return;

	// 新しい曲の再生開始に成功した時点で、既存のBGMボイスを破棄して差し替える
	bgmVoice_ = std::move(voice);
	bgmBaseVolume_ = volume;
	currentBgm_ = sound;
}

void AudioManager::StopBGM() {
	bgmVoice_.reset();
	currentBgm_ = nullptr;
}

void AudioManager::PlaySE(const std::string& filename, float volume) {
	if (const SoundData* sound = GetOrLoadSound(filename)) {
		Voice voice = StartVoice(*sound, volume * seVolume_, false);

		// SEごとにボイスを保持して重ね再生を行い、終了後はUpdateで回収する
		if (voice) {
			seVoices_.push_back({std::move(voice), volume});
		}
	}
}

void AudioManager::SetBGMVolume(float volume) {
	bgmVolume_ = std::clamp(volume, 0.0f, 1.0f);

	// 曲ごとの音量に、設定画面で変更するBGM全体音量を掛けて反映する
	if (bgmVoice_) {
		bgmVoice_->SetVolume(bgmBaseVolume_ * bgmVolume_);
	}
}

void AudioManager::SetSEVolume(float volume) {
	seVolume_ = std::clamp(volume, 0.0f, 1.0f);
	// 各SEの再生時に指定した音量を基準に、SE全体音量を反映する
	for (const auto& se : seVoices_) {
		se.voice->SetVolume(se.volume * seVolume_);
	}
}

std::vector<float> AudioManager::GetSpectrum() const {
	std::vector<float> spectrum(64);

	// スペクトラムは取得されたときにだけ、現在のBGMの再生位置から計算する
	if (bgmVoice_) {
		FFT{}.GetSpectrum(spectrum, *bgmVoice_, *currentBgm_);
	}
	return spectrum;
}
