#pragma once
#include "GameObjects/Player/IPlayerState.h"

namespace Player{
class StateNormal : public IPlayerState{
public:
	// 更新処理
	void Update(float deltaTime)override;
};
}
