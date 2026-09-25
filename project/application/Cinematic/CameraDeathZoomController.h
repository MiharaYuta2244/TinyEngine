#pragma once
#include "GameTimer.h"
#include "Vector3.h"

/// <summary>
/// プレイヤー死亡時、カメラの演出用クラス
/// </summary>
class CameraDeathZoomController {
public:
	/// <summary>
	/// 演出の開始処理
	/// </summary>
	/// <param name="startPos">演出開始時のカメラ座標</param>
	/// <param name="startEuler">演出開始時のカメラのオイラー角</param>
	/// <param name="targetPos">XZ方向の最終目標座標</param>
	void Start(const Vector3& startPos, const Vector3& startEuler, const Vector3& targetPos);

	// カメラの座標を更新して返す
	Vector3 Update(float deltaTime);

	// 演出用に計算された現在の回転を取得
	const Vector3& GetRotation() const { return currentEuler_; }

	// アクティブ状態かどうか
	bool GetIsActive() const { return isActive_; }

	// 演出が終了したかどうか
	bool GetIsFinished() const { return isFinished_; }

	// カメラ演出を強制的に終わらせる
	void Skip();

private:
	// 切り替わる回数
	int maxStep_ = 4;

	// 現在の段階
	int step_ = 0;

	// Y座標の変化量
	float addAmount_ = 180.0f;

	// 段階が切り替わる時間
	GameTimer changeTimer_;

	// 現在のY座標
	float currentY_ = 0.0f;

	// アクティブフラグ
	bool isActive_ = false;

	// 終了フラグ
	bool isFinished_ = false;

	// 切り替わる間隔
	float changeDuration_ = 1.0f;

	// XZ移動・回転演出用
	Vector3 startPos_{};     // 演出開始時の座標
	Vector3 targetPos_{};    // XZの最終目標座標
	Vector3 startEuler_{};   // 演出開始時の回転
	Vector3 targetEuler_{};  // 最終的な回転
	Vector3 currentEuler_{}; // 現在の回転

	float totalElapsed_ = 0.0f; // 演出全体の経過時間
};