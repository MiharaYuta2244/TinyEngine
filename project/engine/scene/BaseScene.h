#pragma once
#include "DebugCamera.h"
#include "TimeManager.h"
#include "DirectInput.h"
#include "EngineContext.h"
#include "GamePad.h"

class SceneManager;

/// <summary>
/// シーンの基底クラス
/// </summary>
class BaseScene {
public:
	virtual ~BaseScene() = default;

	// シーン初期化
	virtual void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, Camera* debugCamera, TimeManager* timeManager, SceneManager* sceneManager) = 0;

	// シーン更新
	virtual void Update() = 0;

	// シーン描画
	virtual void Draw() = 0;

	// シーン終了処理
	virtual void Finalize() = 0;

	virtual void RequestSceneChange(const std::string& sceneName);

protected:
	EngineContext* engineContext_ = nullptr;
	DirectInput* keyboard_ = nullptr;
	GamePad* gamePad_ = nullptr;
	Camera* mainCamera_ = nullptr;
	TimeManager* timeManager_ = nullptr;
	SceneManager* sceneManager_ = nullptr;
};
