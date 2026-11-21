#include "PlayerAnimator.h"
#include "AnimationLibrary.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include "ModelManager.h"

void PlayerAnimator::Initialize(ObjectRender* render, ModelManager* modelManager) {
	render_ = render;
	current_ = PlayerAnim::Idle;
	clothed_ = false;
	actionTimer_ = 0.0f;

	// 掴み続けるポーズの両手の中心を、描画と同じモデル座標で求める
	const char* file = ToFile(PlayerAnim::HoldIdle);
	auto* model = modelManager->FindModel(file);
	const auto& root = model->GetModelData().rootNode;
	auto skeleton = model->CreateSkeleton(root);
	model->ApplyAnimation(skeleton, AnimationLibrary::Get(file), 0.0f);
	model->UpdateSkeleton(skeleton);

	const auto palm = [&](const char* jointName) {
		const auto& joint = skeleton.joints[skeleton.jointMap.at(jointName)];
		auto jointMatrix = joint.skeletonSpaceMatrix * root.localMatrix;
		Vector3 palmOffset = {0.0f, 0.55f, 0.0f};
		return MathUtility::Transform(palmOffset, jointMatrix);
	};

	barricadeGripOffset_ = (palm("ボーン.009") + palm("ボーン.018")) * 0.5f;
}

void PlayerAnimator::SetClothed(bool clothed) {
	if (clothed_ == clothed) {
		return;
	}

	clothed_ = clothed;
	render_->SetModel(clothed_ ? "GorillaClothed.gltf" : ToFile(current_));
}

void PlayerAnimator::Update(float dt, bool isMoving, bool isHolding, bool isPullingBarricade) {
	if (isPullingBarricade) {
		actionTimer_ = 0.0f;
		Play(PlayerAnim::HoldIdle, 0.0f);
		return;
	}

	if (actionTimer_ > 0.0f) {
		actionTimer_ -= dt;
		return; // アクション中は上書きしない
	}

	// プレイヤーの状態に応じてアニメーションを再生
	PlayerAnim target = isHolding ? (isMoving ? PlayerAnim::HoldWalk : PlayerAnim::HoldIdle) : (isMoving ? PlayerAnim::Walk : PlayerAnim::Idle);
	Play(target);
}

void PlayerAnimator::Play(PlayerAnim anim, float blendDuration) {
	if (anim == current_)
		return;

	current_ = anim;
	const char* file = ToFile(anim);
	if (!clothed_) {
		render_->SetModel(file);
	}
	render_->GetObject3d()->PlayAnimation(AnimationLibrary::Get(file), blendDuration);
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
