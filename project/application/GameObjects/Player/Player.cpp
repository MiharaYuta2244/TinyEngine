#include "Player.h"
#include "GameObjects/Enemy/Enemy.h"

void Player::Initialize(EngineContext* ctx) {
	transform_.scale = {1.0f, 1.0f, 1.0f};
	transform_.rotate = {0.0f, 0.0f, 0.0f};
	transform_.translate = {0.0f, 0.0f, 0.0f};

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "suzanne.obj");
	render_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

	// 移動用インスタンス生成
	move_ = std::make_unique<PlayerMove>();

	// HP管理用インスタンス生成&初期化
	hp_ = std::make_unique<PlayerHealth>();
	hp_->Initialize(maxHP_);
}

void Player::Update(float deltaTime, DirectInput* input, Enemy* enemy) {
	bool right = input->KeyDown(DIK_D);
	bool left = input->KeyDown(DIK_A);
	bool front = input->KeyDown(DIK_W);
	bool back = input->KeyDown(DIK_S);

	// 入力方向ベクトル
	Vector2 dir = {0.0f, 0.0f};

	if (right)
		dir.x += 1.0f;
	if (left)
		dir.x -= 1.0f;
	if (front)
		dir.y += 1.0f;
	if (back)
		dir.y -= 1.0f;

	// 移動方向があれば記録
	if (dir.x != 0.0f || dir.y != 0.0f) {
		lastMoveDirection_ = MathUtility::Normalize(dir);
	}

	// 移動更新
	move_->Update(&transform_, dir, deltaTime);

	// 描画更新
	render_->Update(transform_);

	// HP管理インスタンス更新
	hp_->Update(deltaTime);

	// HP管理用インスタンスImGui
	hp_->DrawImGui();

	// 攻撃用の当たり判定更新
	Vector3 pos = transform_.translate;
	attackCol_.max = {pos.x + 0.5f, pos.y, pos.z + 0.5f};
	attackCol_.min = {pos.x - 0.5f, pos.y, pos.z - 0.5f};

	if (enableAttack_ && input->KeyDown(DIK_J)) {
		isHold_ = true;
		enemy->SetPos(pos);
		enemy->SetEnableMove(false);
	}

	if (isHold_ && input->KeyReleased(DIK_J)) {
		isHold_ = false;
		enemy->SetEnableMove(true);
	}

	if (enableAttack_ && input->KeyTriggered(DIK_K)) {
		isHold_ = false;
		enableAttack_ = false;
		enemy->SetEnableMove(true);
		enemy->StartKnockBack({lastMoveDirection_.x, 0.0f, lastMoveDirection_.y});
	}
}

void Player::Draw() {
	// 描画
	render_->Draw();
}

bool Player::IsDead() const { return hp_->IsDead(); }

void Player::Damage(float value) { hp_->Damage(value); }
