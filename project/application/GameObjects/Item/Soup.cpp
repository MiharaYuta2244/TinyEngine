#include "Soup.h"

void Soup::Inititlize(EngineContext* ctx){
	render_ = std::make_unique<ObjectRender>();
	render_->Initialize(ctx, "Soup.obj");
}

void Soup::Update(float deltaTime) {
	render_->Update();
}

void Soup::Draw(){
	render_->Draw();
}