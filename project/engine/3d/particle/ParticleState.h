#include "Transform.h"
#include "Vector2.h"
#include "Vector4.h"

/// <summary>
/// 汎用モジュールが個体ごとに持つ値
/// </summary>
struct ParticleState {
	Transform transform;
	Vector2 uvScroll;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;

	// GenericModule用
	Vector3 startScale = {1.0f, 1.0f, 1.0f};
	Vector4 startColor = {1.0f, 1.0f, 1.0f, 1.0f};
	float spin = 0.0f;
};