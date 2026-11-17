#include "Molotov.h"
#include "Collision.h"
#include "EffectLibrary.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include "Particle.h"
#include <algorithm>
#include <cmath>

Molotov::Molotov() = default;
Molotov::~Molotov() = default;

void Molotov::Initialize(EngineContext* ctx, const Transform& transform) {
	ctx_ = ctx;
	transform_ = transform;
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Molotov.obj");
	render_->SetTransform(transform_);
}

void Molotov::Throw(const Vector3& direction) {
	if (!CanGrab() || MathUtility::LengthSquared(direction) < 0.0001f) {
		return;
	}

	constexpr float horizontalSpeed = 16.0f;
	velocity_ = MathUtility::Normalize(direction) * horizontalSpeed;
	velocity_.y = 8.0f;
	transform_.translate.y = std::max(transform_.translate.y, kGroundY) + 0.8f;
	inFlight_ = true;
}

void Molotov::Update(float deltaTime) {
	if (inFlight_) {
		constexpr float gravity = 22.0f;
		const float height = transform_.translate.y - kGroundY;
		const float timeToGround = (velocity_.y + std::sqrt(velocity_.y * velocity_.y + 2.0f * gravity * height)) / gravity;
		const float step = std::min(deltaTime, timeToGround);
		previousPos_ = transform_.translate;
		transform_.translate += velocity_ * step;
		transform_.translate.y -= 0.5f * gravity * step * step;
		velocity_.y -= gravity * step;
		transform_.rotate.x += 8.0f * step;
		grounded_ = deltaTime >= timeToGround;
		if (grounded_) {
			transform_.translate.y = kGroundY;
		}
	}

	fireTime_ = std::max(0.0f, fireTime_ - deltaTime);
	for (auto& particle : particles_) {
		particle->Update();
	}
	std::erase_if(particles_, [](const auto& particle) { return particle->IsFinished(); });

	if (!broken_) {
		render_->Update(transform_);
		UpdateAABBForGizmo();
	}
}

bool Molotov::CheckThrowHit(const AABB& obstacle, float& time) const {
	const Vector3 movement = transform_.translate - previousPos_;
	return Collision::Intersect(Segment{previousPos_, movement}, obstacle, time);
}

void Molotov::BlockThrow(float time) {
	// 障害物の手前で水平移動を止め、地面に落下させる。
	const Vector3 movement = transform_.translate - previousPos_;
	const float backoff = 0.15f / std::max(0.15f, MathUtility::Length(movement));
	transform_.translate.x = previousPos_.x + movement.x * std::max(0.0f, time - backoff);
	transform_.translate.z = previousPos_.z + movement.z * std::max(0.0f, time - backoff);
	velocity_.x = 0.0f;
	velocity_.z = 0.0f;
	render_->Update(transform_);
}

void Molotov::LandIfGrounded() {
	if (grounded_) {
		Ignite();
	}
}

void Molotov::Ignite() {
	inFlight_ = false;
	broken_ = true;
	fireTime_ = kFireDuration;

	EffectLibrary::Get().Spawn(ctx_, "MolotovFire", transform_.translate, particles_);
	for (auto& particle : particles_) {
		particle->Update();
	}
}

bool Molotov::OverlapsFire(const AABB& body) const {
	const Vector3& center = transform_.translate;
	const float dx = center.x - std::clamp(center.x, body.min.x, body.max.x);
	const float dz = center.z - std::clamp(center.z, body.min.z, body.max.z);
	return dx * dx + dz * dz <= kFireRadius * kFireRadius &&
	    body.min.y <= center.y + 2.0f && body.max.y >= center.y - 0.5f;
}

bool Molotov::UpdatePlayerExposure(bool inside, float deltaTime) {
	if (!inside) {
		playerExposure_ = 0.0f;
		damagedPlayer_ = false;
		return false;
	}

	playerExposure_ += deltaTime;
	return !damagedPlayer_ && playerExposure_ >= kDamageDelay;
}

void Molotov::Draw() {
	if (!broken_) {
		render_->Draw();
	}
	for (auto& particle : particles_) {
		particle->Draw();
	}
}
