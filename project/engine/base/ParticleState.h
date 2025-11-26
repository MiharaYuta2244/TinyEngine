#pragma once
#include "Transform.h"
#include "Vector4.h"

struct ParticleState
{
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};