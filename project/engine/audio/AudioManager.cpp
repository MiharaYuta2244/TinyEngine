#include "AudioManager.h"
#include "StringUtility.h"
#include <algorithm>
#include <mfapi.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <mfreadwrite.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

using namespace Microsoft::WRL;
using namespace TinyEngine;

AudioManager::~AudioManager() {
	std::erase(instances_, this);
	HRESULT result;

	// 再生中のボイスを停止・破棄
	StopBGM();
	for (const auto& se : seVoices_) {
		auto* voice = se.voice;
		if (voice) {
			voice->Stop();
			voice->DestroyVoice();
		}
	}
	seVoices_.clear();

	result = MFShutdown();
	assert(SUCCEEDED(result));

	if (masterVoice_) masterVoice_->DestroyVoice();
	xAudio2_.Reset();

	// マップ内のデータを全て解放
	for (auto& pair : soundData_) {
		SoundUnLoad(&pair.second);
	}
	soundData_.clear();

	spectrum_.clear();
}

void AudioManager::Initialize() {
	HRESULT result;

	result = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	result = xAudio2_->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(result));
	instances_.push_back(this);

	// Windows Media Foundation初期化
	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(result));

	// FFT
	fft_ = std::make_unique<FFT>();

	// スペクトラムのリサイズ
	spectrum_.resize(64);
}

void AudioManager::Update() {
	// 再生が終了したSEのボイスをリストから削除・破棄する
	seVoices_.remove_if([](const SEVoice& se) {
		auto* voice = se.voice;
		XAUDIO2_VOICE_STATE state;
		voice->GetState(&state);
		// バッファのキューが空＝再生終了
		if (state.BuffersQueued == 0) {
			voice->DestroyVoice();
			return true; // リストから削除
		}
		return false; // リストに残す
	});

	// FFT
	if (bgmVoice_) fft_->GetSpectrum(spectrum_, *bgmVoice_, currentBgmTag_, soundData_);
}

void AudioManager::LoadWave(const std::string& tag, const std::string& filename) {
	// 既に同じタグがあれば読み込まない
	if (soundData_.find(tag) != soundData_.end()) {
		return;
	}

	// フルパスをワイド文字列に変換
	std::wstring filePathW = StringUtility::ConvertString(filename);
	HRESULT result;

	// SourceReader作成
	ComPtr<IMFSourceReader> pReader;
	result = MFCreateSourceReaderFromURL(filePathW.c_str(), nullptr, &pReader);
	assert(SUCCEEDED(result));

	// PCM形式にフォーマット指定する
	ComPtr<IMFMediaType> pPCMType;
	MFCreateMediaType(&pPCMType);
	pPCMType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pPCMType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	result = pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pPCMType.Get());
	assert(SUCCEEDED(result));

	// 実際にセットされたメディアタイプを取得する
	ComPtr<IMFMediaType> pOutType;
	pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutType);

	// Waveフォーマットを取得する
	WAVEFORMATEX* waveFormat = nullptr;
	MFCreateWaveFormatExFromMFMediaType(pOutType.Get(), &waveFormat, nullptr);

	// データを一時保管
	SoundData soundData = {};
	soundData.wfex = *(WAVEFORMATEXTENSIBLE*)waveFormat;

	// 生成したwaveフォーマットを開放
	CoTaskMemFree(waveFormat);

	// PCMデータのバッファを構築
	while (true) {
		ComPtr<IMFSample> pSample;
		DWORD streamIndex = 0, flags = 0;
		LONGLONG llTimeStamp = 0;
		// サンプルを読み込む
		result = pReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &flags, &llTimeStamp, &pSample);
		// ストリームの末尾に達したら抜ける
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
			break;

		if (pSample) {
			ComPtr<IMFMediaBuffer> pBuffer;
			// サンプルに含まれるサウンドデータのバッファを一繋ぎにして取得
			pSample->ConvertToContiguousBuffer(&pBuffer);

			BYTE* pData = nullptr; // データ読み取りポインタ
			DWORD maxLength = 0, currentLength = 0;
			// バッファ読み込み用にロック
			pBuffer->Lock(&pData, &maxLength, &currentLength);
			// バッファの末尾にデータを追加
			soundData.buffer.insert(soundData.buffer.end(), pData, pData + currentLength);
			pBuffer->Unlock();
		}
	}

	// マップに登録
	soundData_[tag] = soundData;
}

void AudioManager::PlayBGM(const std::string& tag, float volume) {
	// タグが見つからなければ何もしない
	if (soundData_.find(tag) == soundData_.end()) {
		return;
	}

	// 既に再生中のBGMがあれば停止・破棄
	StopBGM();

	// 現在のタグを保存
	currentBgmTag_ = tag;

	HRESULT result;
	SoundData& data = soundData_[tag];

	// SourceVoiceの生成
	result = xAudio2_->CreateSourceVoice(&bgmVoice_, &data.wfex.Format);
	assert(SUCCEEDED(result));

	// バッファの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = data.buffer.data();
	buf.AudioBytes = static_cast<UINT32>(data.buffer.size());
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// ★BGM用に無限ループ設定
	buf.LoopCount = XAUDIO2_LOOP_INFINITE;

	// 音声データの送信と再生
	result = bgmVoice_->SubmitSourceBuffer(&buf);
	bgmBaseVolume_ = volume;
	bgmVoice_->SetVolume(volume * bgmVolume_);
	result = bgmVoice_->Start();
}

void AudioManager::StopBGM() {
	if (bgmVoice_) {
		bgmVoice_->Stop();
		bgmVoice_->DestroyVoice();
		bgmVoice_ = nullptr;
	}

	// タグ情報をクリア
	currentBgmTag_.clear();
}

void AudioManager::PlaySE(const std::string& tag, float volume) {
	// タグが見つからなければ何もしない
	if (soundData_.find(tag) == soundData_.end()) {
		return;
	}

	HRESULT result;
	SoundData& data = soundData_[tag];

	// SourceVoiceの生成（SEは毎回新しく作る）
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &data.wfex.Format);
	assert(SUCCEEDED(result));

	// バッファの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = data.buffer.data();
	buf.AudioBytes = static_cast<UINT32>(data.buffer.size());
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// SEはループしない (LoopCount = 0)
	buf.LoopCount = 0;

	// 再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	pSourceVoice->SetVolume(volume * seVolume_);
	result = pSourceVoice->Start();

	// リストに追加（Updateで終了監視するため）
	seVoices_.push_back({pSourceVoice, volume});
}

void AudioManager::SetBGMVolume(float volume) {
	bgmVolume_ = std::clamp(volume, 0.0f, 1.0f);
	for (auto* manager : instances_) {
		if (manager->bgmVoice_) manager->bgmVoice_->SetVolume(manager->bgmBaseVolume_ * bgmVolume_);
	}
}

void AudioManager::SetSEVolume(float volume) {
	seVolume_ = std::clamp(volume, 0.0f, 1.0f);
	for (auto* manager : instances_) {
		for (const auto& se : manager->seVoices_) se.voice->SetVolume(se.volume * seVolume_);
	}
}

void AudioManager::SoundUnLoad(SoundData* soundData) {
	soundData->buffer.clear();
	soundData->wfex = {};
}
