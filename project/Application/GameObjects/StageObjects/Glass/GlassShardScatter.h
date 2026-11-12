#pragma once
#include "DecalManager.h"
#include "Vector3.h"
#include <vector>

/// <summary>
/// ガラス破壊時に、デカールの破片を飛ばして散らすクラス
/// </summary>
class GlassShardScatter {
public:
	// 初期化処理
	void Initialize(TinyEngine::DecalManager* decalManager);

	// 更新処理
	void Update(float deltaTime);

	/// <summary>
	/// 破片を生成して飛ばす
	/// </summary>
	/// <param name="center">ガラスの中心</param>
	/// <param name="halfExtent">ガラスの半径(XZ)</param>
	/// <param name="hitDir">叩いた方向(XZ平面、正規化不要)</param>
	void Spawn(const Vector3& center, const Vector3& halfExtent, const Vector3& hitDir);

private:
	struct Shard {
		TinyEngine::DecalManager::DecalData* decal = nullptr;
		Vector3 start{};
		Vector3 end{};
		float yaw = 0.0f;  // 最終的な向き
		float spin = 0.0f; // 飛行中に加える回転量
		float elapsed = 0.0f;
		float duration = 0.3f;
		Vector3 baseScale{};
	};

private:
	TinyEngine::DecalManager* decalManager_ = nullptr;
	std::vector<Shard> shards_;

	// 調整用パラメータ
	int shardCount_ = 70;        // 破片の数
	float maxDistance_ = 6.0f;   // 最大飛距離
	float minSize_ = 0.04f;      // 最小サイズ
	float maxSize_ = 0.22f;      // 最大サイズ
	float coneHalfAngle_ = 1.0f; // 前方に飛ぶ扇の半角
	float forwardRatio_ = 0.7f;  // 叩いた方向へ飛ぶ破片の割合
};