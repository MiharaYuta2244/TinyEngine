#pragma once
#include "BaseScene.h"
#include "GameObjects/CommonData.h"
#include "Fade/SceneTransition.h"
#include <memory>
#include <unordered_map>
#include <vector>

// 遷移の種類
enum class TransitionType {
	None,
	Change,
	Push,
	Pop,
};

/// <summary>
/// シーンマネージャークラス
/// </summary>
class SceneManager {
public:
	SceneManager() = default;
	~SceneManager() = default;

	// シーン初期化（共通リソースを渡す）
	void Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, Camera* debugCamera, TimeManager* timeManager, FadeManager* fadeManager, SceneTransition* sceneTransition);

	// シーン追加
	void AddScene(const std::string& sceneName, std::unique_ptr<BaseScene> scene);

	// シーン切り替え要求
	void ChangeScene(const std::string& sceneName);
	void PushScene(const std::string& sceneName);
	void PopScene();

	// 現在のシーンを取得
	BaseScene* GetCurrentScene() const;

	// 共通データへのポインタを取得
	CommonData* GetCommonData() const { return commonData_.get(); }

	// シーン更新
	void Update();

	// シーン描画
	void Draw();

	// 終了処理
	void Finalize();

	// 未登録のシーンを弾く
	bool HasScene(const std::string& name) const { return scenes_.contains(name); }

	// シーン切り替え中かどうか
	bool HasPendingTransition() const { return requestedTransition_ != TransitionType::None; }

private:
	// シーンマップ
	std::unordered_map<std::string, std::unique_ptr<BaseScene>> scenes_;

	std::vector<BaseScene*> sceneStack_;
	std::string nextSceneName_;

	// 切り替えフラグ
	TransitionType requestedTransition_ = TransitionType::None;

	// 共通リソースへのポインタ
	EngineContext* engineContext_ = nullptr;
	DirectInput* keyboard_ = nullptr;
	GamePad* gamePad_ = nullptr;
	Camera* debugCamera_ = nullptr;
	TimeManager* timeManager_ = nullptr;
	FadeManager* fadeManager_ = nullptr;

	// シーン間で共有するデータ
	std::unique_ptr<CommonData> commonData_;

	// シーン切り替えポインタ
	SceneTransition* sceneTransition_ = nullptr;
};
