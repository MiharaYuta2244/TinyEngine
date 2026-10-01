#pragma once
#include "AABB.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "GameObjects/PlaceableObject.h"
#include "GameTimer.h"

/// <summary>
/// プレイヤーが掴んで投げられるアイテム
/// </summary>
class Soap : public PlaceableObject {
public:
	Soap();

	// 初期化処理
	void Initialize(EngineContext* ctx, const Transform& transform);

	// 更新処理
	void Update(float deltaTime);

	// 描画処理
	void Draw() override;

	// 投げ処理
	void Throw(const Vector3& velocity);

	// 本体の当たり判定Getter
	AABB GetCollision() const { return collision_; }

	// 泡エリアの当たり判定Getter
	AABB GetBubbleArea() const { return bubbleArea_; }

	// 泡が現在有効かどうか
	bool GetIsBubbleActive() const { return isBubbleActive_; }

	// 直接ぶつけて即ダウンさせられる勢いがあるか
	bool IsDangerous() const;

	// 座標のGetter/Setter
	Vector3 GetPos() const { return transform_.translate; }
	void SetPos(Vector3 pos) { transform_.translate = pos; }

	// ギズモ用
	std::string GetName() const override { return "Soup(" + std::to_string(id_) + ")"; }

	// アウトライン有効フラグSetter
	void SetEnableOutline(bool isEnable) override;

	// プレイヤーに掴まれた時の処理
	void Grab();

	// 壁などにぶつかった時の処理
	void Stop();

	// 投げられて飛んでいる最中かどうか
	bool IsThrown() const { return isThrown_; }

private:
	// 速度・摩擦の適用
	void UpdateMove(float deltaTime);

	// 当たり判定の更新
	void UpdateCollision();

	// 泡エリアの発生・更新
	void UpdateBubble(float deltaTime);

private:
	std::unique_ptr<ObjectRender> render_;       // 本体描画
	std::unique_ptr<ObjectRender> bubbleRender_; // 泡の描画

	AABB collision_{};  // 本体の当たり判定
	AABB bubbleArea_{}; // 泡の当たり判定

	Vector3 velocity_ = {0.0f, 0.0f, 0.0f}; // 移動速度
	float friction_ = 4.0f;                 // 減衰率
	float stopThreshold_ = 0.3f;            // これ以下で停止扱い

	Vector3 halfSize_ = {0.5f, 0.3f, 0.5f};       // 本体当たり判定の半径
	Vector3 bubbleHalfSize_ = {2.0f, 0.5f, 2.0f}; // 泡の当たり判定の半径

	bool isThrown_ = false;       // 投げられて飛んでいる最中か
	bool isBubbleActive_ = false; // 泡が発生しているか
	GameTimer bubbleTimer_;       // 泡の持続時間タイマー
	float bubbleDuration_ = 4.0f; // 泡の持続時間

	static constexpr float kDangerousSpeedSq_ = 16.0f; // 直接ヒットで即ダウンさせる速度閾値

	// オブジェクト数カウント用
	static int index;
	int id_ = 0;

	// 1フレーム前の座標
	Vector3 prevPos_ = {0.0f, 0.0f, 0.0f};
};