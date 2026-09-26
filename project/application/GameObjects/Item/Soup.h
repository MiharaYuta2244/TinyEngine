#pragma once
#include "GameObjects/ObjectRender/ObjectRender.h"

class Soup {
public:
	void Inititlize(EngineContext* ctx);

	void Update(float deltaTime);

	void Draw();

private:
	std::unique_ptr<ObjectRender> render_;
};
