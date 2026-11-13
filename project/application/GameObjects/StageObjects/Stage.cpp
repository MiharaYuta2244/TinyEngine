#include "Stage.h"
#include "CameraForGPU.h"
#include "DirectXUtils.h"
#include "FogParam.h"
#include "TimeParam.h"
#include <cmath>
#include <cstddef>
#include <cstring>
#include "JsonManager.h"
#include "MathUtility.h"
#include "MathOperator.h"
#include <algorithm>

#ifdef USE_IMGUI
#include "imgui.h"
#endif

using namespace TinyEngine;

namespace {
struct DecorationConstants {
	alignas(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT) Material material{};
	alignas(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT) CameraForGPU camera{};
	alignas(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT) FogParam fog{};
	alignas(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT) TimeParam time{};
};
}

void Stage::Initialize(const SceneContext& ctx, TinyEngine::DecalManager* decalManager, const std::string& stagePath) {
	// デカール管理インスタンスの受け渡し
	decalManager_ = decalManager;
	engineContext_ = ctx.engineContext;
	decorationManager_ = std::make_unique<DecorationManager>();
	decorationManager_->Initialize(engineContext_, "DecorationEditor", stagePath + "Decorations.json", Transform{{2, 2, 2}, {}, {}});

	// 壁の管理インスタンス生成&初期化
	wallManager_ = std::make_unique<WallManager>();
	wallManager_->Initialize(
	    ctx.engineContext, "WallEditor", stagePath + "Walls.json",
	    Transform{
	        {1, 50, 1},
            {0, 0,  0},
            {0, 0,  0}
    });

	barricadeManager_ = std::make_unique<BarricadeManager>();
	barricadeManager_->Initialize(
	    ctx.engineContext, "BarricadeEditor", stagePath + "Barricades.json",
	    Transform{
	        {1.5f, 2.0f, 0.25f},
            {},
            {}
    });

	// ドアの管理インスタンス生成&初期化
	doorManager_ = std::make_unique<DoorManager>();
	doorManager_->Initialize(
	    ctx.engineContext, "DoorEditor", stagePath + "Doors.json",
	    Transform{
	        {1, 49, 1},
            {0, 0,  0},
            {0, 0,  0}
    });

	// ガラスの管理インスタンス生成&初期化
	glassManager_ = std::make_unique<GlassManager>();
	glassManager_->Initialize(
	    ctx.engineContext, "GlassEditor", stagePath + "Glasses.json",
	    Transform{
	        {1, 50, 1},
            {0, 0,  0},
            {0, 0,  0}
    },
	    decalManager_);

	// ガラスの破片クラス生成&初期化
	glassShardScatter_ = std::make_unique<GlassShardScatter>();
	glassShardScatter_->Initialize(decalManager_);

	// 檻の管理インスタンス生成&初期化
	cageManager_ = std::make_unique<CageManager>();
	cageManager_->Initialize(
	    ctx.engineContext, "CageEditor", stagePath + "Cages.json",
	    Transform{
	        {1, 50, 1},
            {0, 0,  0},
            {0, 0,  0}
    });

	// 回復エリアの管理インスタンス生成&初期化
	healAreaManager_ = std::make_unique<HealAreaManager>();
	healAreaManager_->Initialize(
	    ctx.engineContext, "HealAreaEditor", stagePath + "HealAreas.json",
	    Transform{
	        {1, 50, 1},
            {0, 0,  0},
            {0, 0,  0}
    });

	// SoapManagerの生成&初期化
	soapManager_ = std::make_unique<SoapManager>();
	soapManager_->Initialize(
	    ctx.engineContext, "SoapEditor", stagePath + "Soap.json",
	    Transform{
	        {1, 1, 1},
            {0, 0, 0},
            {0, 0, 0}
    });

	// 地面の生成&初期化
	ground_ = std::make_unique<Ground>();
	ground_->Initialize(ctx.engineContext);

	// ゴール判定インスタンス生成&初期化
	goalPositionPath_ = stagePath + "Goal.json";
	Vector3 goalPosition{0.0f, 0.0f, 0.0f};
	LoadGoalPosition(goalPosition);
	goal_ = std::make_unique<Goal>();
	goal_->Initialize(ctx.engineContext, goalPosition);
	goalEditorMessage_.clear();
}

bool Stage::LoadGoalPosition(Vector3& position) const {
	JsonManager::json data;
	if (!JsonManager::Load(goalPositionPath_, data) ||
	    !data.is_object() || !data.contains("x") || !data["x"].is_number() ||
	    !data.contains("y") || !data["y"].is_number() ||
	    !data.contains("z") || !data["z"].is_number()) {
		return false;
	}

	const Vector3 loadedPosition = data.get<Vector3>();
	if (!MathUtility::IsFinite(loadedPosition)) {
		return false;
	}

	position = loadedPosition;
	return true;
}

void Stage::Update(float deltaTime, const Vector3& playerPos, Camera* camera) {
	wallManager_->Update(deltaTime);
	barricadeManager_->Update(deltaTime);
	doorManager_->Update(deltaTime, playerPos);
	glassManager_->Update();
	glassShardScatter_->Update(deltaTime);
	cageManager_->Update(deltaTime);
	healAreaManager_->Update(deltaTime);
	soapManager_->Update(deltaTime);
	ground_->Update();
	goal_->Update(deltaTime);
}

void Stage::Draw(Camera* camera) {
	ground_->Draw();
	doorManager_->Draw();
	wallManager_->Draw();
	barricadeManager_->Draw();
	cageManager_->Draw();
	soapManager_->Draw();
	goal_->Draw();
	if (camera) {
		DrawDecorations(*camera);
	}
}

void Stage::DrawDecorations(Camera& camera) {
	const auto& decorations = decorationManager_->GetObjects();
	if (decorations.empty()) {
		return;
	}

	for (auto& [model, instances] : decorationBatches_) {
		instances.clear();
	}

	const auto& viewProjection = camera.GetViewProjectionMatrix();
	for (const auto& decoration : decorations) {
		const auto world = decoration->GetModelWorldMatrix();
		const float determinant = MathUtility::Dot(
		    Vector3{world.m[0][0], world.m[0][1], world.m[0][2]},
		    MathUtility::Cross({world.m[1][0], world.m[1][1], world.m[1][2]}, {world.m[2][0], world.m[2][1], world.m[2][2]}));
		const auto normalMatrix = std::abs(determinant) > 1e-6f
		    ? MathUtility::Transpose(MathUtility::Inverse(world))
		    : MathUtility::MakeIdentity4x4();
		decorationBatches_[decoration->GetModel()].push_back({world * viewProjection, world, normalMatrix});
	}

	auto* common = engineContext_->object3dCommon;
	auto* dx = common->GetDxCommon();
	const auto device = dx->GetDevice();

	// フレーム末尾でGPUの完了を待つため、次フレームで同じアップロードバッファを再利用できる。
	if (decorations.size() > decorationInstanceCapacity_) {
		const size_t capacity = std::max(decorations.size(), decorationInstanceCapacity_ * 2);
		auto buffer = DirectXUtils::CreateBufferResource(device, capacity * sizeof(TransformationMatrix));
		if (!buffer) {
			return;
		}
		decorationInstanceBuffer_ = std::move(buffer);
		decorationInstanceCapacity_ = capacity;
	}
	if (!decorationConstants_) {
		decorationConstants_ = DirectXUtils::CreateBufferResource(device, sizeof(DecorationConstants));
		if (!decorationConstants_) {
			return;
		}
	}

	DecorationConstants constants{};
	constants.material.color = {1, 1, 1, 1};
	constants.material.uvTransform = MathUtility::MakeIdentity4x4();
	constants.material.enableLighting = true;
	constants.material.shininess = 64.0f;
	constants.camera.worldPosition = camera.GetTranslation();

	const D3D12_RANGE noRead{0, 0};
	void* mappedConstants = nullptr;
	if (FAILED(decorationConstants_->Map(0, &noRead, &mappedConstants))) {
		return;
	}
	std::memcpy(mappedConstants, &constants, sizeof(constants));
	decorationConstants_->Unmap(0, nullptr);

	void* mappedInstances = nullptr;
	if (FAILED(decorationInstanceBuffer_->Map(0, &noRead, &mappedInstances))) {
		return;
	}

	common->DrawSettingCommon(engineContext_->textureManager, true);
	const auto commandList = dx->GetCommandList();
	const auto constantsAddress = decorationConstants_->GetGPUVirtualAddress();
	commandList->SetGraphicsRootConstantBufferView(0, constantsAddress + offsetof(DecorationConstants, material));
	commandList->SetGraphicsRootConstantBufferView(4, constantsAddress + offsetof(DecorationConstants, camera));
	commandList->SetGraphicsRootConstantBufferView(5, constantsAddress + offsetof(DecorationConstants, fog));
	commandList->SetGraphicsRootConstantBufferView(6, constantsAddress + offsetof(DecorationConstants, time));

	const auto instanceAddress = decorationInstanceBuffer_->GetGPUVirtualAddress();
	size_t byteOffset = 0;
	for (const auto& [model, instances] : decorationBatches_) {
		if (instances.empty()) {
			continue;
		}
		const size_t batchBytes = instances.size() * sizeof(TransformationMatrix);
		std::memcpy(static_cast<char*>(mappedInstances) + byteOffset, instances.data(), batchBytes);
		commandList->SetGraphicsRootShaderResourceView(Object3dCommon::kInstanceDataRootIndex, instanceAddress + byteOffset);
		model->Draw("", nullptr, static_cast<UINT>(instances.size()));
		byteOffset += batchBytes;
	}
	decorationInstanceBuffer_->Unmap(0, nullptr);
}

void Stage::DrawTransparent() {
	glassManager_->Draw();
	healAreaManager_->Draw();
}

void Stage::DrawImGui() {
	wallManager_->DrawImGui();
	decorationManager_->DrawImGui();
	barricadeManager_->DrawImGui();
	doorManager_->DrawImGui();
	glassManager_->DrawImGui();
	cageManager_->DrawImGui();
	healAreaManager_->DrawImGui();
	soapManager_->DrawImGui();

#ifdef USE_IMGUI
	if (ImGui::Begin("Goal")) {
		ImGui::TextUnformatted(goalPositionPath_.c_str());
		auto& position = goal_->GetTransform().translate;
		Vector3 editedPosition = position;
		if (ImGui::DragFloat3("Position", &editedPosition.x, 0.1f) && MathUtility::IsFinite(editedPosition)) {
			position = editedPosition;
			goalEditorMessage_.clear();
		}

		if (ImGui::Button("Save")) {
			const bool saved = MathUtility::IsFinite(position) && JsonManager::Save(goalPositionPath_, position);
			goalEditorMessage_ = saved ? "Saved." : "Could not save goal position.";
		}
		ImGui::SameLine();
		if (ImGui::Button("Load")) {
			goalEditorMessage_ = LoadGoalPosition(position) ? "Loaded." : "Could not load goal position.";
		}
		ImGui::TextUnformatted(goalEditorMessage_.c_str());
	}
	ImGui::End();
#endif
}

std::vector<AABB> Stage::GetSolidObstacles() const {
	const auto& walls = wallManager_->GetObjects();
	const auto& barricades = barricadeManager_->GetObjects();
	std::vector<AABB> obstacles;
	const auto& decorations = decorationManager_->GetObjects();
	obstacles.reserve(walls.size() + barricades.size() + decorations.size());

	for (const auto& wall : walls) {
		obstacles.push_back(wall->GetCollision());
	}

	for (const auto& decoration : decorations) {
		obstacles.push_back(decoration->GetCollision());
	}

	for (const auto& barricade : barricades) {
		if (barricade->IsSolid()) {
			obstacles.push_back(barricade->GetCollision());
		}
	}

	return obstacles;
}
