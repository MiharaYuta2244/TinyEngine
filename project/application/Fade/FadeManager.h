#pragma once
#include "IFadeState.h"
#include "Sprite.h"
#include "EngineContext.h"
#include <memory>
#include <string>

class FadeManager {
public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="ctx">エンジンコンテキスト</param>
	void Initialize(EngineContext* ctx);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="deltaTime">経過時間</param>
	void Update(float deltaTime);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// フェードアウト開始要求
	/// </summary>
	/// <param name="sceneName"></param>
	void FadeOutTo(const std::string& sceneName);

	/// <summary>
	/// Gameクラスからシーン切り替えを完了したときに呼ぶ
	/// </summary>
	void NotifySceneChanged();

	/// <summary>
	/// Stateパターン用の状態変更
	/// </summary>
	/// <param name="newState">新しい状態</param>
	void ChangeState(std::unique_ptr<IFadeState> newState);

	/// <summary>
	/// フェードの透明度を設定する
	/// </summary>
	/// <param name="alpha">透明度</param>
	void SetFadeAlpha(float alpha);

	// Getter Setter
	bool IsWaitingForSceneShange() const { return isWaitingForSceneChange_; }
	void SetWaitingForSceneChange(bool waiting) { isWaitingForSceneChange_ = waiting; }

	const std::string& GetRequestedSceneName() const { return requestedSceneName_; }

	float GetFadeTimer() const { return fadeTimer_; }
	void SetFadeTimer(float timer) { fadeTimer_ = timer; }
	void AddFadeTimer(float deltaTime) { fadeTimer_ += deltaTime; }
	float GetFadeDuration() const { return fadeDuration_; }

private:
	std::unique_ptr<TinyEngine::Sprite> fadeSprite_;
	std::unique_ptr<IFadeState> currentState_;

	float fadeDuration_ = 1.0f; // フェード時間
	float fadeTimer_ = 0.0f;    // カウント用のタイマー

	std::string requestedSceneName_;       // フェードアウト開始時に切り替えるシーン名
	bool isWaitingForSceneChange_ = false; // シーン切り替え待ちフラグ
};
