#pragma once
#include "PostEffectSetting.h"

/// <summary>
/// ステージごとの記録
/// </summary>
struct StageRecord {
	bool cleared = false;  // クリア済みか
	float bestTime = 0.0f; // 最速クリアタイム
	int maxKillCount = 0;  // 最大撃破数
};

/// <summary>
/// シーンをまたいで保持するデータ
/// </summary>
struct CommonData {
	static constexpr int kStageCount = 3;   // ステージ数
	int killCount = 0;                      // 敵の死亡数
	float clearTime = 0.0f;                 // クリアタイム
	bool isClear = false;                   // クリアしたかどうか
	std::string currentStageKey = "Stage1"; // 現在プレイ中のステージ
	int currentStageNo = 1;                 // 現在のステージ番号
	PostEffectSettings postEffectSettings;  // ポストエフェクトのON/OFF設定
	
	// ステージごとの記録
	std::array<StageRecord, kStageCount> stageRecords;

	// クリア時に記録を更新する
	void ReportClear(int stageNo, float time, int kills) {
		int i = stageNo - 1;
		if (i < 0 || i >= kStageCount) {
			return;
		}
		StageRecord& r = stageRecords[i];
		r.bestTime = r.cleared ? std::min(r.bestTime, time) : time;
		r.maxKillCount = std::max(r.maxKillCount, kills);
		r.cleared = true;
	}
};
