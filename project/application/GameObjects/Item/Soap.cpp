#include "Soap.h"
#include "GameObjects/Enemy/Enemy.h"
#include "EffectLibrary.h"
#include "MathUtility.h"
#include "Particle.h"
#include <algorithm>
#include <numbers>

int Soap::index = 0;

Soap::Soap() { id_ = index++; }

Soap::~Soap() = default;

void Soap::Initialize(EngineContext* ctx, const Transform& transform) {
	ctx_ = ctx;
	transform_ = transform;

	// 本体描画
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Soup.obj");
	render_->SetTransform(transform_);

	UpdateCollision();
}

void Soap::Update(float deltaTime) {
	// 移動処理
	UpdateMove(deltaTime);

	// 当たり判定更新
	UpdateCollision();
	UpdateAABBForGizmo();

	// 泡エリアの更新
	UpdateBubble(deltaTime);

	// 描画用インスタンス更新
	render_->SetTransform(transform_);
	render_->Update();
}

void Soap::Draw() {
	// 本体描画
	if (!isBroken_) {
		render_->Draw();
	}

	// 泡エフェクト描画
	for (auto& particle : bubbleEffect_) {
		particle->Draw();
	}
}

void Soap::Throw(const Vector3& velocity) {
	prevPos_ = transform_.translate;

	// ベクトルの長さが十分あれば正規化して設定したスピードを適用
	if (MathUtility::LengthSquared(velocity) > 0.0001f) {
		velocity_ = MathUtility::Scale(MathUtility::Normalize(velocity), throwSpeed_);
	} else {
		velocity_ = {0.0f, 0.0f, 0.0f};
	}

	isThrown_ = true;
	StopBubble(); // 投げ直された場合は前回の泡を消す
}

bool Soap::IsDangerous() const {
	float speedSq = velocity_.x * velocity_.x + velocity_.z * velocity_.z;
	return isThrown_ && speedSq >= kDangerousSpeedSq_;
}

void Soap::UpdateMove(float deltaTime) {
	prevPos_ = transform_.translate;

	if (!isThrown_) {
		return;
	}

	transform_.translate.x += velocity_.x * deltaTime;
	transform_.translate.z += velocity_.z * deltaTime;

	// 摩擦による減速
	velocity_.x = MathUtility::Lerp(velocity_.x, 0.0f, friction_ * deltaTime);
	velocity_.z = MathUtility::Lerp(velocity_.z, 0.0f, friction_ * deltaTime);

	// 十分減速したら停止扱いにして泡を発生させる
	if (std::abs(velocity_.x) < 0.1f && std::abs(velocity_.z) < 0.1f) {
		velocity_ = {0.0f, 0.0f, 0.0f};
		isThrown_ = false;

		if (!isBubbleActive_) {
			isBubbleActive_ = true;

			if (!isBroken_) {
				justBroken_ = true;
			}

			isBroken_ = true;
			bubbleTimer_.Initialize(bubbleDuration_);
		}
	}
}

void Soap::UpdateCollision() {
	Vector3 pos = transform_.translate;
	collision_.max = {pos.x + halfSize_.x, pos.y, pos.z + halfSize_.z};
	collision_.min = {pos.x - halfSize_.x, pos.y, pos.z - halfSize_.z};
}

void Soap::UpdateBubble(float deltaTime) {
	// 泡エリアの当たり判定 
	Vector3 pos = transform_.translate;
	bubbleArea_.max = {pos.x + bubbleHalfSize_.x, pos.y, pos.z + bubbleHalfSize_.z};
	bubbleArea_.min = {pos.x - bubbleHalfSize_.x, pos.y, pos.z - bubbleHalfSize_.z};

	// エフェクトは泡の効果が切れた後も、残った粒が消えるまで更新する
	for (auto& particle : bubbleEffect_) {
		particle->Update();
	}
	std::erase_if(bubbleEffect_, [](const std::unique_ptr<TinyEngine::Particle>& p) { return p->IsFinished(); });

	if (!isBubbleActive_) {
		return;
	}

	bubbleTimer_.Update(deltaTime);
	if (bubbleTimer_.IsEnd()) {
		isBubbleActive_ = false;
	}
}

void Soap::Grab() {
	velocity_ = {0.0f, 0.0f, 0.0f};
	isThrown_ = false;
	StopBubble(); // 持ち上げたら泡は消す
}

void Soap::Stop() {
	transform_.translate = prevPos_;
	velocity_ = {0.0f, 0.0f, 0.0f};
	isThrown_ = false;

	if (!isBroken_) {
		justBroken_ = true;
	}

	isBroken_ = true;
	StartBubble();
}

bool Soap::ConsumeJustBroken() {
	bool result = justBroken_;
	justBroken_ = false;
	return result;
}

void Soap::StartBubble() {
	isBubbleActive_ = true;
	bubbleTimer_.Initialize(bubbleDuration_);
	SpawnBubbleEffect();
}

void Soap::StopBubble() {
	isBubbleActive_ = false;
	bubbleEffect_.clear(); // 掴まれた・投げ直された時は余韻も含めて即消す
}

void Soap::SpawnBubbleEffect() {
	bubbleEffect_.clear();

	// 発生範囲は当たり判定と揃える
	EffectBuilder::SpawnOptions opt;
	opt.extent = Vector3{bubbleHalfSize_.x, 0.0f, bubbleHalfSize_.z};

	EffectLibrary::Get().Spawn(ctx_, "SoapBubble", transform_.translate, bubbleEffect_, opt);

	// 発生を止めるタイミングは泡の持続時間に合わせる
	for (auto& particle : bubbleEffect_) {
		particle->SetEmitMode(false, bubbleDuration_);
	}
}

void Soap::OnCollisionEnter(const CollisionEvent& event, const CollisionContext&) {
	switch (event.other.GetTag()) {
	case CollisionTag::Wall:
	case CollisionTag::Barricade:
	case CollisionTag::Glass:
	case CollisionTag::Cage:
	case CollisionTag::Door:
		if (IsThrown()) {
			Stop();
		}
		break;
	case CollisionTag::Enemy: {
		auto& enemy = static_cast<Enemy&>(event.other);
		if (enemy.IsDead() || enemy.IsDown()) {
			break;
		}
		if (event.selfPart == CollisionPart::Thrown && IsDangerous()) {
			enemy.StartDown();
			Stop();
		} else if (event.selfPart == CollisionPart::Bubble && GetIsBubbleActive()) {
			enemy.StartDown();
			StopBubble();
			isConsumed_ = true;
		}
		break;
	}
	default:
		break;
	}
}
