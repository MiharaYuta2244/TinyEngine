#pragma once
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ParticleCommon.h"
#include "SrvManager.h"

/// <summary>
/// エンジンのコンテキストを保持する構造体
/// </summary>
struct EngineContext {
	Object3dCommon* object3dCommon = nullptr;
	SpriteCommon* spriteCommon = nullptr;
	TextureManager* textureManager = nullptr;
	ModelManager* modelManager = nullptr;
	ParticleCommon* particleCommon = nullptr;
	SrvManager* srvManager = nullptr;
};