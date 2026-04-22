#include "TitleScene.h"
#include "SceneManager.h"

void TitleScene::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, Camera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	mainCamera_ = debugCamera;
	timeManager_ = timeManager;
	sceneManager_ = sceneManager;


}

void TitleScene::Update() {
	if(keyboard_->KeyTriggered(DIK_SPACE)){
		sceneManager_->ChangeScene("GamePlay");
	}
}

void TitleScene::Draw() {

}

void TitleScene::Finalize() {

}
