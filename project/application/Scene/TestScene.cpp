#include "TestScene.h"

void TestScene::Initialize(EngineContext* ctx, DirectInput* keyboard, GamePad* gamePad, DebugCamera* debugCamera, DeltaTime* timeManager, SceneManager* sceneManager) {
	engineContext_ = ctx;
	keyboard_ = keyboard;
	gamePad_ = gamePad;
	debugCamera_ = debugCamera;
	timeManager_ = timeManager;
	sceneManager_ = sceneManager;

	debugCamera_->SetTranslation({0.0f, 0.0f, -10.0f});

	// 球モデル生成&初期化
	sphereModel_ = std::make_unique<Object3d>();
	sphereModel_->Initialize(ctx);
	sphereModel_->SetModel("MonsterBall.obj");
	sphereModel_->SetRotate({0.0f, -std::numbers::pi_v<float> / 2, 0.0f});

	// PlaneObj生成&初期化
	planeObjModel_ = std::make_unique<Object3d>();
	planeObjModel_->Initialize(ctx);
	planeObjModel_->SetModel("plane.obj");
	planeObjModel_->SetTranslate({5.0f, 0.0f, 0.0f});
	planeObjModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});

	// PlaneGltf生成&初期化
	planeGltfModel_ = std::make_unique<Object3d>();
	planeGltfModel_->Initialize(ctx);
	planeGltfModel_->SetModel("plane.gltf");
	planeGltfModel_->SetTranslate({-5.0f, 0.0f, 0.0f});
	planeGltfModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});

	// field生成&初期化
	fieldModel_ = std::make_unique<Object3d>();
	fieldModel_->Initialize(ctx);
	fieldModel_->SetModel("terrain.obj");
	fieldModel_->SetTranslate({0.0f, -2.0f, 0.0f});
	fieldModel_->SetRotate({0.0f, std::numbers::pi_v<float>, 0.0f});
}

void TestScene::Update() {
	sphereModel_->Update();
	planeObjModel_->Update();
	planeGltfModel_->Update();
	fieldModel_->Update();

#ifdef USE_IMGUI
	ImGui::Begin("DirectionalLight");
	ImGui::DragFloat3("Direction", &fieldModel_->GetDirectionalLight().direction.x, 0.01f);
	ImGui::DragFloat("Intensity", &fieldModel_->GetDirectionalLight().intensity, 0.01f);
	ImGui::ColorEdit4("Color", &fieldModel_->GetDirectionalLight().color.x);
	ImGui::End();

	ImGui::Begin("PointLight");
	ImGui::DragFloat3("Position", &fieldModel_->GetPointLight().position.x, 0.01f);
	ImGui::DragFloat("Decay", &fieldModel_->GetPointLight().decay, 0.01f);
	ImGui::DragFloat("Radius", &fieldModel_->GetPointLight().radius, 0.01f);
	ImGui::ColorEdit4("Color", &fieldModel_->GetPointLight().color.x);
	ImGui::DragFloat("Intensity", &fieldModel_->GetPointLight().intensity);
	ImGui::End();

	ImGui::Begin("SpotLight");
	ImGui::DragFloat3("Position", &fieldModel_->GetSpotLight().position.x, 0.01f);
	ImGui::DragFloat3("Direction", &fieldModel_->GetSpotLight().direction.x, 0.01f);
	ImGui::DragFloat("Decay", &fieldModel_->GetSpotLight().decay, 0.01f);
	ImGui::DragFloat("CosAngle", &fieldModel_->GetSpotLight().cosAngle, 0.01f);
	ImGui::DragFloat("Distance", &fieldModel_->GetSpotLight().distance, 0.01f);
	ImGui::ColorEdit4("Color", &fieldModel_->GetSpotLight().color.x);
	ImGui::DragFloat("Intensity", &fieldModel_->GetSpotLight().intensity);
	ImGui::End();

	ImGui::Begin("Field");
	ImGui::DragFloat("Shininess", &fieldModel_->GetMaterial().shininess);
	ImGui::End();

	ImGui::Begin("MonsterBall");
	ImGui::DragFloat3("Scale", &sphereModel_->GetScale().x, 0.01f);
	ImGui::DragFloat("Shininess", &sphereModel_->GetMaterial().shininess);
	ImGui::DragFloat3("Direction", &sphereModel_->GetDirectionalLight().direction.x, 0.01f);
	ImGui::DragFloat("Intensity", &sphereModel_->GetDirectionalLight().intensity, 0.01f);
	ImGui::ColorEdit4("Color", &sphereModel_->GetDirectionalLight().color.x);
	ImGui::End();
#endif
}

void TestScene::Draw() {
	sphereModel_->Draw();
	planeObjModel_->Draw();
	planeGltfModel_->Draw();
	fieldModel_->Draw();
}

void TestScene::Finalize() {}
