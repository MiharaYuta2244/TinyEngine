#pragma once
#include "DirectInput.h"
#include "GamePad.h"
#include "GameTimer.h"

#ifdef USE_IMGUI
#include "imgui.h"
#endif

/// <summary>
/// コントローラーの振動を一括管理するクラス
/// </summary>
class RumbleManager {
public:
	struct RumbleParam {
		float left = 0.0f;     // 強振動 (左モーター)
		float right = 0.0f;    // 弱振動 (右モーター)
		float duration = 0.0f; // 継続時間
	};

	void Initialize(GamePad* gamePad);
	void Update(float deltaTime);

	// 任意のパラメータで振動を開始
	void StartRumble(float left, float right, float duration);
	void StartRumble(const RumbleParam& param);

	// 各イベントごとの振動実行関数
	void TriggerPlayerDamage();
	void TriggerEnemyDeath();
	void TriggerGlassBreak();

	// ImGui描画
	void DrawImGui();

	// 各種パラメータの取得・設定
	RumbleParam& GetPlayerDamageParam() { return playerDamageParam_; }
	RumbleParam& GetEnemyDeathParam() { return enemyDeathParam_; }
	RumbleParam& GetGlassBreakParam() { return glassBreakParam_; }

private:
	GamePad* pad_ = nullptr;
	GameTimer rumbleTimer_;

	// イベント別の振動パラメータ
	RumbleParam playerDamageParam_{0.5f, 0.5f, 0.2f};
	RumbleParam enemyDeathParam_{0.3f, 0.3f, 0.2f};
	RumbleParam glassBreakParam_{0.2f, 0.2f, 0.15f};
};