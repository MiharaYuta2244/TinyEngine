#pragma once
#include <string>

class SceneManager;
class FadeManager;

/// <summary>
/// シーン遷移を担当するクラス
/// </summary>
class SceneTransition {
public:
	// 初期化
	void Initialize(SceneManager* sceneManager, FadeManager* fadeManager);

	// 更新処理
	void Update(float deltaTime);

	// シーン切り替え処理
	void ChangeScene(const std::string& sceneName);

	// フェードなしで即座に切り替え
	void ChangeSceneImmediate(const std::string& sceneName);

	// 現在のシーンの上に重ねる
	void PushScene(const std::string& sceneName);

	// 一番上のシーンを取り除く
	void PopScene();

	// フェードアウト中かどうか
	bool IsFadingOut() const { return state_ == State::FadingOut; }

private:
	enum class State {
		Idle,
		FadingOut,
	};

	SceneManager* sceneManager_ = nullptr;
	FadeManager* fadeManager_ = nullptr;
	State state_ = State::Idle;
};