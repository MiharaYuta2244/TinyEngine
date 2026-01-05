#pragma once
#include "EngineContext.h"
#include "Object3d.h"
#include <memory>

class ResultModel {
public:
	void Initialize(EngineContext* ctx, std::string filename);
	void Update(float deltaTime);
	void Draw();

private:
	std::unique_ptr<Object3d> resultModel_;
};
