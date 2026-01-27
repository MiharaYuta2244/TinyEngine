#include "Orange.h"
#include <numbers>

void Orange::Initialize(EngineContext* ctx, const std::string& fruitName) {
	IFruit::Initialize(ctx,fruitName);
}

void Orange::Update(float deltaTime) {
	IFruit::Update(deltaTime);
}

void Orange::Draw() { IFruit::Draw(); }