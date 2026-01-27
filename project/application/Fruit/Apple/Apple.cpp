#include "Apple.h"
#include <numbers>

void Apple::Initialize(EngineContext* ctx, const std::string& fruitName) { IFruit::Initialize(ctx, fruitName); }

void Apple::Update(float deltaTime) { IFruit::Update(deltaTime); }

void Apple::Draw() { IFruit::Draw(); }