#include "PlayerAnimator.h"
#include "AnimationLibrary.h"

void PlayerAnimator::Initialize(ObjectRender* render) {
	render_ = render;
	current_ = PlayerAnim::Idle;
	clothed_ = false;
	actionTimer_ = 0.0f;
}

void PlayerAnimator::SetClothed(bool clothed) {
	if (clothed_ == clothed) {
		return;
	}

	clothed_ = clothed;
	render_->SetModel(clothed_ ? "GorillaClothed.gltf" : ToFile(current_));
}

void PlayerAnimator::Update(float dt, bool isMoving, bool isHolding) {
	if (actionTimer_ > 0.0f) {
		actionTimer_ -= dt;
		return; // アクション中は上書きしない
	}

	// プレイヤーの状態に応じてアニメーションを再生
	PlayerAnim target = isHolding ? (isMoving ? PlayerAnim::HoldWalk : PlayerAnim::HoldIdle) : (isMoving ? PlayerAnim::Walk : PlayerAnim::Idle);
	Play(target);
}

void PlayerAnimator::Play(PlayerAnim anim) {
	if (anim == current_)
		return;

	current_ = anim;
	const char* file = ToFile(anim);
	if (!clothed_) {
		render_->SetModel(file);
	}
	render_->GetObject3d()->PlayAnimation(AnimationLibrary::Get(file));
}

void PlayerAnimator::PlayAction(PlayerAnim anim, float duration) {
	Play(anim);
	actionTimer_ = duration;
}

const char* PlayerAnimator::ToFile(PlayerAnim anim) {
	switch (anim) {
	case PlayerAnim::Idle:
		return "GorillaIdle.gltf";

	case PlayerAnim::Walk:
		return "Gorilla.gltf";

	case PlayerAnim::HoldIdle:
		return "GorillaHolding.gltf";

	case PlayerAnim::HoldWalk:
		return "GorillaHoldWalking.gltf";

	case PlayerAnim::Hold:
		return "GorillaHold.gltf";

	case PlayerAnim::Push:
		return "GorillaPush.gltf";

	default:
		return "GorillaIdle.gltf";
	}
}
