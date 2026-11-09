#include "DecalManager.h"

/// <summary>
/// プレイヤーが敵を掴んだ時に出る軌道プレビュー
/// </summary>
class ThrowPreview {
public:
	// 初期化処理
	void Initialize(TinyEngine::DecalManager* decalManager);

	// 更新処理
	void Update(float deltaTime, const Vector3& startPos, const Vector2& dir, float power, float friction);

	// 非表示
	void Hide();

private:
	// 軌道をシミュレーションして通過点を返す
	std::vector<Vector3> SimulateTrajectory(Vector3 pos, Vector2 velocity, float friction) const;

	// 通過点の累積距離を計算し、総距離を返す
	static float BuildAccumulatedDistances(const std::vector<Vector3>& points, std::vector<float>& outDistances);

	// 軌道上のdistanceに対応する座標を返す
	static Vector3 SampleTrajectory(const std::vector<Vector3>& points, const std::vector<float>& distances, float distance);

private:
	// 表示するドットの数
	static constexpr int kDotCount = 12;

	// シミュレーション設定
	static constexpr float kSimDeltaTime = 1.0f / 60.0f;
	static constexpr int kMaxSimSteps = 240;
	static constexpr float kStopSpeed = 0.1f;

	// ドット表示設定
	static constexpr float kMaxDotScale = 0.28f;
	static constexpr float kMinDotScale = 0.03f;
	static constexpr float kDotHeight = 0.1f;
	static constexpr float kHiddenY = -1000.0f;

	std::array<TinyEngine::DecalManager::DecalData*, kDotCount> decals_{};

	// スクロール
	float scrollOffset_ = 0.0f;
	float scrollSpeed_ = 4.0f;

	// 色
	Vector4 color_ = {1.0f, 0.85f, 0.2f, 0.8f};
};