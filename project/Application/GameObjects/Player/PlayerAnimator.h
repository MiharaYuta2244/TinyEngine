#include "GameObjects/ObjectRender/ObjectRender.h"

enum class PlayerAnim { Idle, Walk, HoldIdle, HoldWalk, Hold, Push };

/// <summary>
/// プレイヤーのアニメーション管理クラス
/// </summary>
class PlayerAnimator {
public:
	// 初期化関数
	void Initialize(ObjectRender* render);

	// 更新関数
	void Update(float deltaTime, bool isMoving, bool isHolding);

	// アニメーションを一定時間再生
	void PlayAction(PlayerAnim anim, float duration = 0.2f);

private:
	// 再生処理
	void Play(PlayerAnim anim);

	// 状態に応じたファイルパスを返す関数
	static const char* ToFile(PlayerAnim anim);

private:
	ObjectRender* render_ = nullptr;
	PlayerAnim current_ = PlayerAnim::Idle;
	float actionTimer_ = 0.0f;
};