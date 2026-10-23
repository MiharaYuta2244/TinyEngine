#include "SceneTransition.h"
#include "Fade/FadeManager.h"
#include "SceneManager.h"

void SceneTransition::Initialize(SceneManager* sceneManager, FadeManager* fadeManager) {
	sceneManager_ = sceneManager;
	fadeManager_ = fadeManager;
	state_ = State::Idle;
}

void SceneTransition::Update(float deltaTime) {
	// フェードの進行
	fadeManager_->Update(deltaTime);

	// 画面が完全に暗くなったらシーンを切り替える
	if (state_ == State::FadingOut && fadeManager_->IsWaitingForSceneShange()) {
		sceneManager_->ChangeScene(fadeManager_->GetRequestedSceneName());
		fadeManager_->NotifySceneChanged(); // フェードイン開始
		state_ = State::Idle;
	}
}

void SceneTransition::ChangeScene(const std::string& sceneName) {
	// フェードアウト中の二重要求、未登録シーン、SceneManager側で遷移待ちの場合は無視
	if (state_ == State::FadingOut || !sceneManager_->HasScene(sceneName) || sceneManager_->HasPendingTransition()) {
		return;
	}

	state_ = State::FadingOut;
	fadeManager_->FadeOutTo(sceneName);
}

void SceneTransition::ChangeSceneImmediate(const std::string& sceneName) { sceneManager_->ChangeScene(sceneName); }

void SceneTransition::PushScene(const std::string& sceneName) {
	if (state_ == State::FadingOut) {
		return;
	}

	sceneManager_->PushScene(sceneName);
}

void SceneTransition::PopScene() {
	if (state_ == State::FadingOut) {
		return;
	}

	sceneManager_->PopScene();
}