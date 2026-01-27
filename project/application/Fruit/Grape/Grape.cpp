#include "Grape.h"
#include <numbers>

void Grape::Initialize(EngineContext* ctx, const std::string& fruitName) { IFruit::Initialize(ctx, fruitName); }

void Grape::Update(float deltaTime) { IFruit::Update(deltaTime); }

void Grape::Draw() { IFruit::Draw(); }