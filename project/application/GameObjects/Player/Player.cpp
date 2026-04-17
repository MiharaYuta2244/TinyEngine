#include "Player.h"

void Player::Initialize(EngineContext* ctx) {
	transform_.scale = {1.0f,1.0f,1.0f};
	transform_.rotate = {0.0f,0.0f,0.0f};
	transform_.translate = {0.0f,0.0f,0.0f};

	// 描画用インスタンス生成&初期化
	render_ = std::make_unique<PlayerRender>();
	render_->Initialize(ctx);

	// 移動用インスタンス生成
	move_=std::make_unique<PlayerMove>();
}

void Player::Update(float deltaTime, DirectInput* input) {
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

	// 移動更新
	move_->Update(&transform_, dir, deltaTime);

	// 描画更新
	render_->Update(transform_);
}

void Player::Draw(){
	// 描画
	render_->Draw();
}

void Player::Damage(int value){

}