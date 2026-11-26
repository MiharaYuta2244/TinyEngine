#pragma once
#include <cmath>
#include <complex>
#include <vector>

struct SoundData;
struct IXAudio2SourceVoice;

/// <summary>
/// FFTクラス
/// </summary>
class FFT {
public:
	// BGMの再生位置付近のPCMをFFTで解析し、周波数成分を出力する。
	void GetSpectrum(std::vector<float>& output, IXAudio2SourceVoice& bgmVoice, const SoundData& data);
};
