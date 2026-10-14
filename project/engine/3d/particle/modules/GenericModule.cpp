#include "GenericModule.h"
#include "ColorPalette.h"
#include "Easing.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include "Random.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace {
float Rand(const FloatRange& r) { return RandomUtils::RangeFloat(std::min(r.min, r.max), std::max(r.min, r.max)); }
constexpr float kDeg2Rad = std::numbers::pi_v<float> / 180.0f;
} // namespace

void GenericModule::Initialize(ParticleState& particle, EngineContext* /*ctx*/) {
	particle.lifeTime = std::max(Rand(p_.life), 0.001f);
	particle.currentTime = 0.0f;

	// サイズ
	float s = Rand(p_.startSize);
	particle.startScale = {s * p_.sizeAxis.x, s * p_.sizeAxis.y, s * p_.sizeAxis.z};
	particle.transform.scale = particle.startScale;

	// 回転
	particle.transform.rotate = MathUtility::DegreeToRadian(p_.initialRotateDeg);
	particle.spin = Rand(p_.spinDeg) * kDeg2Rad;

	// 色
	float t = RandomUtils::RangeFloat(0.0f, 1.0f);
	particle.startColor = ColorPalette::Lerp(p_.colorStartA, p_.colorStartB, t);
	particle.color = particle.startColor;

	// 初速
	float speed = Rand(p_.speed);
	float phi = RandomUtils::RangeFloat(0.0f, 2.0f * std::numbers::pi_v<float>);

	if (p_.shape == EmitShape::CircleXZ) {
		particle.velocity = {std::cos(phi) * speed, Rand(p_.yVelocity), std::sin(phi) * speed};
		return;
	}

	// directionを中心にspreadAngleの範囲へ飛ばす
	Vector3 d = MathUtility::Normalize(p_.direction);
	Vector3 up = std::abs(d.y) < 0.99f ? Vector3{0.0f, 1.0f, 0.0f} : Vector3{1.0f, 0.0f, 0.0f};
	Vector3 right = MathUtility::Normalize(MathUtility::Cross(up, d));
	Vector3 up2 = MathUtility::Cross(d, right);

	float cosMax = std::cos(std::clamp(p_.spreadAngle, 0.0f, 180.0f) * kDeg2Rad);
	float cosT = RandomUtils::RangeFloat(cosMax, 1.0f);
	float sinT = std::sqrt(std::max(0.0f, 1.0f - cosT * cosT));

	Vector3 dir = d * cosT + (right * std::cos(phi) + up2 * std::sin(phi)) * sinT;
	particle.velocity = dir * speed;
}

void GenericModule::Update(ParticleState& particle, float deltaTime, EngineContext* /*ctx*/) {
	float t = std::clamp(particle.currentTime / particle.lifeTime, 0.0f, 1.0f);

	// 重力・減衰
	particle.velocity += p_.gravity * deltaTime;
	particle.velocity *= std::pow(p_.damping, deltaTime * 60.0f);

	// サイズ
	float k = MathUtility::Lerp(1.0f, p_.endScale, Easing::ApplyEasing(p_.sizeEase, t));
	particle.transform.scale = particle.startScale * k;

	// 色
	particle.color = ColorPalette::Lerp(particle.startColor, p_.colorEnd, Easing::ApplyEasing(p_.colorEase, t));

	// 回転
	particle.transform.rotate.z += particle.spin * deltaTime;
}