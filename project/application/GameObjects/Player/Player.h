#pragma once
#include "AABB.h"
#include "AudioManager.h"
#include "DecalManager.h"
#include "DirectInput.h"
#include "GameObjects/IGameObject.h"
#include "GameObjects/Item/Soap.h"
#include "GameObjects/ObjectRender/ObjectRender.h"
#include "GameTimer.h"
#include "OBB.h"
#include "Particle.h"
#include "PlayerAnimator.h"
#include "PlayerGrab.h"
#include "PlayerHealth.h"
#include "PlayerInput.h"
#include "PlayerMove.h"
#include "ThrowPreview.h"
#include "Utility/GameRumbleController.h"

class EnemyManager;
class Enemy;

/// <summary>
/// プレイヤーの処理をまとめたクラス
/// </summary>
class Player : public IGameObject {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, TinyEngine::DecalManager* bloodDecalManager);

	// 更新処理
	void Update(float deltaTime, DirectInput* input, GamePad* gamePad, EnemyManager* enemyManager, const std::list<std::unique_ptr<Soap>>& soaps);

	// 当たり判定などの解決後に行う最終更新処理
	void PostUpdate();

	// 描画処理
	void Draw();

	// 座標のGetter
	Vector3 GetPosition() const { return transform_.translate; }
	Vector3& GetPosition() { return transform_.translate; }

	// 座標のSetter
	void SetPosition(const Vector3& pos) { transform_.translate = pos; }

	// 死亡フラグGetter
	bool IsDead() const;

	// ダメージ処理
	void Damage(float value);

	// 回復処理
	void Heal(float value);

	// 全回復処理
	void AllHeal();

	// 攻撃用の当たり判定Getter
	OBB GetAttackCol() const { return attackCol_; }

	// 攻撃可能かどうかSetter
	void SetEnableAttack(bool enableAttack) { enableAttack_ = enableAttack; }

	// 本体の当たり判定
	AABB GetBodyCol() const { return bodyCol_; }

	// 敵のポインタ
	Enemy* GetTargetEnemy() const { return grab_->GetHeldEnemy(); }

	// 現在のHP取得Getter
	float GetCurrentHP() const { return hp_->GetCurrentHP(); }

	// HPの最大値取得Getter
	float GetMaxHP() const { return hp_->GetMaxHP(); }

	// 描画用のオブジェクトを返すGetter
	TinyEngine::Object3d* GetObject3d() { return render_->GetObject3d(); }

	// 回転のGetter
	Vector3 GetRotation() const { return transform_.rotate; }

	// プレイヤーが移動中かどうかを取得するGetter
	bool IsMoving() const { return isMoving_; }

	// ギズモ用
	std::string GetName() const override { return "Player"; }

	// プレイヤーの向いている方向のGetter
	Vector2 GetDirection() const { return lastMoveDirection_; }

	// 敵を掴んでいるかどうかGetter
	bool IsGrabbingEnemy() const { return grab_->IsHoldingEnemy(); }

	// 掴んでいる敵のポインタGetter
	Enemy* GetGrabbedEnemy() { return grab_->GetHeldEnemy(); }

	// 攻撃フラグGetter
	bool GetIsAttack() const { return isAttackTriggered_; }

	// プレイヤーが敵を掴んだかどうか
	bool GetIsGrabTriggered() const { return isGrabTriggered_; }
	void SetIsGrabTriggered(bool isGrabTriggered) { isGrabTriggered_ = isGrabTriggered; }

	// ImGui描画
	void DrawImGui();

	// 振動マネージャーGetter
	GameRumbleController* GetRumbleManager() { return rumbleManager_.get(); }

	// 死亡している敵のポインタを破棄
	void ReleaseGrabbedEnemy() { grab_->ReleaseEnemyByGuard(); }

private:
	// 当たり判定の更新処理
	void UpdateCollision();

	// ヒットエフェクト生成関数
	void GenerateHitEffect();

	// 回復エフェクトの生成関数
	void GenerateHealEffect();

	// 血痕の生成
	void AddBloodDecal(Vector3 scale);

	// HPが2以下の時の出血処理
	void Bleeding(float deltaTime);

	// 投げ軌道プレビューの更新
	void UpdateThrowPreview(float deltaTime);

private:
	Vector2 velocity_;
	OBB attackCol_;
	AABB bodyCol_;
	Vector2 lastMoveDirection_;
	EngineContext* ctx_ = nullptr;

	// 色
	Vector4 color_ = {1.0f, 0.1568f, 0.0f, 1.0f};

	// プレイヤーの最大HP
	float maxHP_ = 3.0f;

	// 攻撃可能かどうかを表す変数
	bool enableAttack_ = false;

	std::unique_ptr<ObjectRender> render_;       // 描画用インスタンス
	std::unique_ptr<PlayerMove> move_;           // 移動用インスタンス
	std::unique_ptr<PlayerHealth> hp_;           // HP管理用インスタンス
	std::unique_ptr<PlayerInput> input_;         // 入力用インスタンス
	std::unique_ptr<PlayerAnimator> animator_;   // アニメーション用インスタンス
	std::unique_ptr<ThrowPreview> throwPreview_; // 軌道プレビュー用インスタンス
	std::unique_ptr<PlayerGrab> grab_;           // プレイヤーアクション用インスタンス

	// 現在移動中かどうかを保持するフラグ
	bool isMoving_ = false;

	// 敵を掴んでいるときにかける速度倍率
	float speedMultiplier_ = 1.0f;

	// 被弾時エフェクト
	std::vector<std::unique_ptr<TinyEngine::Particle>> hitEffects_;
	float preHP_ = maxHP_;

	// 回復エフェクト
	std::vector<std::unique_ptr<TinyEngine::Particle>> healEffects_;

	// 敵を掴んだ瞬間のフラグ
	bool isGrabTriggered_ = false;

	// 攻撃しているかどうか
	bool isAttackTriggered_ = false;

	// 血痕の管理インスタンスポインタ
	TinyEngine::DecalManager* bloodDecalManager_ = nullptr;

	// 出血の間隔用タイマー
	GameTimer bleedingTimer_;

	// 出血の間隔
	float bleedingInterval_ = 0.2f;

	// プレイヤーの生存時間
	float aliveTImer_ = 0.0f;

	// 前フレームのLTボタンの入力値
	float preLt_ = 0.0f;

	// 前フレームのRTボタンの入力値
	float preRt_ = 0.0f;

	// ポイントライト調整用パラメータ
	float pointLightHeightOffset_ = 2.0f;                // プレイヤー中心からの高さ
	Vector4 pointLightColor_ = {1.0f, 1.0f, 1.0f, 1.0f}; // ライトの色
	float pointLightIntensity_ = 0.0f;                   // 輝度
	float pointLightRadius_ = 7.0f;                      // 影響半径
	float pointLightDecay_ = 0.7f;                       // 減衰率

	// オーディオマネージャー
	std::unique_ptr<TinyEngine::AudioManager> audioManager_;

	// 振動マネージャー
	std::unique_ptr<GameRumbleController> rumbleManager_;

	// 攻撃インターバル
	GameTimer attackTimer_;
};