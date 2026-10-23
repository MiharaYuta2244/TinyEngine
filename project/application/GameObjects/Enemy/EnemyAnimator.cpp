#include "EnemyAnimator.h"
#include "AnimationLibrary.h"
#include "Object3d.h"

void EnemyAnimator::Initialize(TinyEngine::Object3d* object3d) {
	object3d_ = object3d;
	hasPlayed_ = false;
	current_ = EnemyAnim::Idle;

	// 初回再生時のヒッチを避けるため、全クリップを先に読み込んでおく
	for (const char* file : kClipFiles) {
		AnimationLibrary::Preload(file);
	}

	// 初期状態は待機。ブレンドなしで即座に開始する
	Play(EnemyAnim::Idle, 0.0f);
}

void EnemyAnimator::Update(EnemyState state, bool isMoving) { Play(Select(state, isMoving)); }

void EnemyAnimator::Play(EnemyAnim anim, float blendDuration) {
	if (!object3d_) {
		return;
	}

	// すでに同じアニメーションを再生中なら何もしない
	if (hasPlayed_ && anim == current_) {
		return;
	}

	current_ = anim;
	hasPlayed_ = true;

	const char* file = kClipFiles[static_cast<std::size_t>(anim)];
	object3d_->PlayAnimation(AnimationLibrary::Get(file), blendDuration);
}

EnemyAnim EnemyAnimator::Select(EnemyState state, bool isMoving) {
	switch (state) {
	case EnemyState::Vigilance:
		// 警戒中は停止していても警戒歩きモーション
		return EnemyAnim::VigilanceWalk;

	case EnemyState::Hold:
	case EnemyState::Down:
		return EnemyAnim::Idle;

	case EnemyState::Normal:
	default:
		return isMoving ? EnemyAnim::Walk : EnemyAnim::Idle;
	}
}
