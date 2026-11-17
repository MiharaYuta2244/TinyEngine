#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include <list>
#include <memory>

class Enemy;
class EnemyManager;
class Soap;
class Molotov;
class ThrowableItem;
class Barricade;

/// <summary>
/// プレイヤーの「掴む・投げる・突き飛ばす」を担当するクラス
/// </summary>
class PlayerGrab {
public:
	// 今フレームの入力
	struct Input {
		bool grabHeld = false;      // 掴みボタンを押している
		bool grabPressed = false;   // 掴みボタンを押した瞬間
		bool grabReleased = false;  // 掴みボタンを離した瞬間
		bool attackPressed = false; // 攻撃ボタンを押した瞬間
		Vector2 moveDir{};
	};

	// プレイヤー側の情報
	struct Context {
		Vector3 position{};
		Vector3 rotate{};
		Vector2 forward{0.0f, 1.0f}; // XZ平面の向き
		bool enableAttack = false;   // 攻撃可能か
		float deltaTime = 0.0f;
	};

	// プレイヤー側が演出などに使う結果
	struct Result {
		bool grabbedEnemy = false; // 敵を掴んだ瞬間
		bool grabbedItem = false;  // 投擲アイテムを掴んだ瞬間
		bool grabbedBarricade = false;
		bool attacked = false;     // 攻撃を行った
		bool stopMovement = false; // プレイヤーの速度を0にしてほしい
	};

	// 更新処理
	Result Update(
	    const Input& input, const Context& ctx, EnemyManager* enemyManager, const std::list<std::unique_ptr<Soap>>& soaps,
	    const std::list<std::unique_ptr<Molotov>>& molotovs,
	    const std::list<std::unique_ptr<Barricade>>& barricades);

	void ValidateBarricade(const std::list<std::unique_ptr<Barricade>>& barricades);
	void ReleaseBarricade();
	void PlaceShield(const Vector3& position, const Vector2& forward, float yaw);
	bool IsPullingBarricade() const;
	bool IsHoldingShield() const;
	bool IsHoldingBarricade() const { return heldBarricade_ != nullptr; }

	// 被弾ガードなど、外部要因で掴んでいる敵を手放す
	void ReleaseEnemyByGuard();

	// 投げ軌道プレビューに必要な物理パラメータを取得する
	bool GetPreviewParams(bool enableAttack, Vector3& outPos, float& outPower, float& outFriction) const;

	// 何かを掴んでいるか
	bool IsHolding() const { return heldEnemy_ != nullptr || heldItem_ != nullptr || IsHoldingBarricade(); }

	// 敵を掴んでいるか
	bool IsHoldingEnemy() const { return heldEnemy_ != nullptr; }

	// 掴んでいる敵
	Enemy* GetHeldEnemy() const { return heldEnemy_; }

private:
	// 掴める一番近い敵・投擲アイテムを探す
	float FindTargets(const Context& ctx, EnemyManager* enemyManager, const std::list<std::unique_ptr<Soap>>& soaps, const std::list<std::unique_ptr<Molotov>>& molotovs);

	// 投擲アイテムの掴み・投げ・追従
	void UpdateItem(const Input& input, const Context& ctx, Result& result);

	// 敵を掴む
	void TryGrabEnemy(const Input& input, const Context& ctx, Result& result);

	// 掴んでいる敵の追従・解放
	void UpdateHeldEnemy(const Input& input, const Context& ctx);

	// 敵の突き飛ばし・投げ
	void UpdateAttack(const Input& input, const Context& ctx, Result& result);

	// 敵を通常状態に戻して手放す
	void ReleaseEnemy();

private:
	Enemy* heldEnemy_ = nullptr; // 掴んでいる敵
	ThrowableItem* heldItem_ = nullptr;   // 掴んでいる投擲アイテム
	Barricade* heldBarricade_ = nullptr;

	Enemy* nearEnemy_ = nullptr; // 今フレーム掴める敵
	ThrowableItem* nearItem_ = nullptr;   // 今フレーム掴める投擲アイテム

	float grabRange_ = 3.0f;  // 掴める距離
	float holdOffset_ = 1.5f; // 掴んでいる間、プレイヤーの前方に置く距離
};
