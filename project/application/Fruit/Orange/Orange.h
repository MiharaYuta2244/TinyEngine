#pragma once
#include "Fruit/IFruit.h"

class Orange : public IFruit {
public:
	void Initialize(EngineContext* ctx, const std::string& fruitName) override;
	void Update(float deltaTime) override;
	void Draw() override;
};
