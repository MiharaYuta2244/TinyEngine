#include "D3DResourceLeakChecker.h"
#include "DebugCamera.h"
#include "DirectInput.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "MathUtility.h"
#include "Model.h"
#include "ModelCommon.h"
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "WinApp.h"
#include "XAudio.h"
#include "ModelManager.h"
#include <memory>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// リリースリークチェック
	D3DResourceLeakChecker leakCheck;

	// ウィンドウ作成
	auto winApp = std::make_shared<WinApp>();

	// DirectX12 デバイス初期化
	auto dxCommon = std::make_unique<DirectXCommon>();
	dxCommon->Initialize(winApp);

	// ImGuiManager
	std::unique_ptr<ImGuiManager> imGuiManager_ = std::make_unique<ImGuiManager>();
	imGuiManager_->Initialize(dxCommon->GetWinApp()->GetHWND(), dxCommon->GetDevice(), dxCommon->GetSwapChainDescBufferCount(), dxCommon->GetRtvFormat(), dxCommon->GetSrvDescriptorHeap().Get());

	// テクスチャマネージャー
	std::unique_ptr<TextureManager> textureManager_ = std::make_unique<TextureManager>();
	textureManager_->Initialize(dxCommon.get());

	// Object3dCommon
	auto object3dCommon = std::make_unique<Object3dCommon>();
	object3dCommon->Initialize(dxCommon.get());

	// ModelManager
	auto modelManger = std::make_unique<ModelManager>();
	modelManger->Initialize(dxCommon.get(), textureManager_.get());

	// Object3d
	std::vector<std::unique_ptr<Object3d>> object3ds;
	for (uint32_t i = 0; i < 5; ++i) {
		auto object3d = std::make_unique<Object3d>();
		object3d->Initialize(object3dCommon.get(), textureManager_.get(), modelManger.get());
		object3ds.push_back(std::move(object3d));
	}

	// .objファイルからモデルを読み込む
	modelManger->LoadModel("fence.obj");
	modelManger->LoadModel("plane.obj");
	modelManger->LoadModel("axis.obj");
	modelManger->LoadModel("SkySphere.obj");
	modelManger->LoadModel("skydome.obj");
	modelManger->LoadModel("Field.obj");
	modelManger->LoadModel("sphere.obj");

	// modelのポインタを受け取る
	object3ds[0]->SetModel("fence.obj");
	object3ds[1]->SetModel("skydome.obj");
	object3ds[2]->SetModel("SkySphere.obj");
	object3ds[3]->SetModel("sphere.obj");
	object3ds[4]->SetModel("Field.obj");

	object3ds[1]->SetEnableFoging(false);
	object3ds[2]->SetEnableFoging(false);
	object3ds[3]->SetEnableFoging(false);
	object3ds[4]->SetEnableFoging(false);

	object3ds[1]->SetColor({1.0f, 1.0f, 1.0f, 0.5f});
	object3ds[3]->SetColor({1.0f, 1.0f, 1.0f, 0.5f});

	// Sprite共通部
	auto spriteCommon = std::make_unique<SpriteCommon>();
	spriteCommon->Initialize(dxCommon.get());

	// Sprite
	std::vector<std::unique_ptr<Sprite>> sprites;
	for (uint32_t i = 0; i < 5; ++i) {
		auto sprite = std::make_unique<Sprite>();
		sprite->Initialize(spriteCommon.get(), textureManager_.get(), "resources/uvChecker.png");
		sprites.push_back(std::move(sprite));
	}

	// XAudio
	std::unique_ptr<XAudio> audio = std::make_unique<XAudio>();
	audio->Initialize();
	audio->SoundsAllLoad();

	// DirectInput
	std::unique_ptr<DirectInput> input = std::make_unique<DirectInput>();
	input->Initialize(hInstance, dxCommon->GetWinApp()->GetHWND());

	// DebugCamera
	std::unique_ptr<DebugCamera> debugCamera = std::make_unique<DebugCamera>();
	debugCamera->Initialize();

	// model座標
	Transform transform[5];

	// color
	Vector4 color = {1.0f,1.0f,1.0f,1.0f};

	// 音声再生
	// audio->SoundPlayWave(audio->GetXAudio2().Get(), audio->GetSound());

	// ウィンドウのxボタンが押されるまでループ
	while (dxCommon->GetWinApp()->ProcessMessage()) {

		input->Update();

		imGuiManager_->BeginFrame();

		debugCamera->Update(*input);
		for (uint32_t i = 0; i < 5; ++i) {
			object3ds[i]->SetViewMatrix(debugCamera->GetViewMatrix());
		}

		for (uint32_t i = 0; i < 5; ++i) {
			transform[i].translate = object3ds[i]->GetTranslate();
			transform[i].rotate = object3ds[i]->GetRotate();
			transform[i].scale = object3ds[i]->GetScale();
		}

		// ImGui
		ImGui::DragFloat3("translate0", &transform[0].translate.x, 0.01f);
		ImGui::DragFloat3("translate1", &transform[1].translate.x, 0.01f);
		ImGui::DragFloat3("translate2", &transform[2].translate.x, 0.01f);
		ImGui::DragFloat3("translate3", &transform[3].translate.x, 0.01f);
		ImGui::DragFloat3("translate4", &transform[4].translate.x, 0.01f);
		ImGui::DragFloat3("rotate0", &transform[0].rotate.x, 0.01f);
		ImGui::DragFloat3("rotate1", &transform[1].rotate.x, 0.01f);
		ImGui::DragFloat3("rotate2", &transform[2].rotate.x, 0.01f);
		ImGui::DragFloat3("rotate3", &transform[3].rotate.x, 0.01f);
		ImGui::DragFloat3("rotate4", &transform[4].rotate.x, 0.01f);
		ImGui::DragFloat3("scale0", &transform[0].scale.x, 0.01f);
		ImGui::DragFloat3("scale1", &transform[1].scale.x, 0.01f);
		ImGui::DragFloat3("scale2", &transform[2].scale.x, 0.01f);
		ImGui::DragFloat3("scale3", &transform[3].scale.x, 0.01f);
		ImGui::DragFloat3("scale4", &transform[4].scale.x, 0.01f);
		
		ImGui::SliderFloat4("color", &color.x, 0.0f, 1.0f);
		object3ds[3]->SetColor(color);

		for (uint32_t i = 0; i < 5; ++i) {
			object3ds[i]->SetTranslate(transform[i].translate);
			object3ds[i]->SetRotate(transform[i].rotate);
			object3ds[i]->SetScale(transform[i].scale);
		}

		// object3dUpdate
		for (uint32_t i = 0; i < 5; ++i) {
			object3ds[i]->Update();
		}

		// spriteUpdate
		for (uint32_t i = 0; i < 5; ++i) {
			sprites[i]->Update();
		}

		// Sprite ImGui
		Vector2 position = sprites[0]->GetPosition();
		ImGui::DragFloat2("position", &position.x, 0.1f);
		sprites[0]->SetPosition(position);

		Vector2 size = sprites[0]->GetSize();
		ImGui::DragFloat2("size", &size.x, 0.1f);
		sprites[0]->SetSize(size);

		// 描画開始
		dxCommon->BeginFrame();

		// Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
		spriteCommon->DrawSettingCommon();

		// 3Dオブジェクト描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
		object3dCommon->DrawSettingCommon();

		// 3Dモデル描画
		for (uint32_t i = 0; i < 5; ++i) {
			object3ds[i]->Draw();
		}

		// Sprite描画
		//for (uint32_t i = 0; i < 5; ++i) {
		//	sprites[i]->Draw();
		//}

		// ImGuiの内部コマンドを生成する
		imGuiManager_->Render(dxCommon->GetCommandList());

		// 描画終了
		dxCommon->EndFrame();
	}

	imGuiManager_->Finalize();

	CloseHandle(dxCommon->GetFenceEvent());

	CoUninitialize();
	return 0;
}