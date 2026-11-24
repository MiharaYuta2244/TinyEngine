#pragma once
#include "DecalManager.h"
#include "Vector3.h"
#include "Vector4.h"
#include <vector>

/// <summary>
/// 足跡デカールを管理するクラス(プールを使い回す)
/// </summary>
class FootprintManager {
public:
	// 初期化処理
	void Initialize(TinyEngine::DecalManager* decalManager);

	// 更新処理(時間経過で薄くして消す)
	void Update(float deltaTime);

	// 足跡の追加
	void Add(const Vector3& pos, float yaw, bool isLeft);

private:
	struct Footprint {
		TinyEngine::DecalManager::DecalData* decal = nullptr;
		float timer = 0.0f;
		bool active = false;
	};

	static constexpr size_t kPoolSize = 256;
	std::vector<Footprint> pool_;
	size_t nextIndex_ = 0;

	float height_ = 0.06f;                          // 固定の高さ
	float holdTime_ = 3.0f;                         // 薄くなり始めるまでの時間
	float fadeTime_ = 4.0f;                         // 薄くなって消えるまでの時間
	float maxAlpha_ = 0.8f;                         // 初期の透明度
	Vector3 scale_ = {0.5f, 0.5f, 1.0f};            // スケール
	Vector4 baseColor_ = {0.4f, 0.25f, 0.1f, 1.0f}; // 色
};