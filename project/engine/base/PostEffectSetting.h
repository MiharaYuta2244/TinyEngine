#pragma once
struct PostEffectSettings {
	bool vignetteEnabled = true;         // ビネット
	bool outlineEnabled = true;          // アウトライン
	bool radialBlurEnabled = true;       // RadialBlur
	bool glitchEnabled = true;           // グリッチ
	bool scanlineEnabled = true;         // 走査線
	bool barrelDistortionEnabled = true; // 魚眼
	bool blurEnabled = true;             // Smoothing + Gaussian
	bool deathEffectEnabled = true;      // 死亡エフェクト（階調表現）
};