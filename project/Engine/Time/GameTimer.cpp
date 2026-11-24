#include "GameTimer.h"

void GameTimer::Initialize(float duration) { 
	timer_ = 0.0f; 
	duration_ = duration;
}

void GameTimer::Update(float deltaTime) { timer_ += deltaTime; }

bool GameTimer::IsEnd() const {
	return duration_ < timer_;
}