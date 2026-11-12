#include "Stage.h"

using namespace TinyEngine;

void Stage::Initialize(const SceneContext& ctx, TinyEngine::DecalManager* decalManager, const std::string& stagePath) {
	// デカール管理インスタンスの受け渡し
	decalManager_ = decalManager;

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
	goal_ = std::make_unique<Goal>();
	goal_->Initialize(ctx.engineContext);

	// ステージごとのゴール初期配置
	if (stagePath == "Stage1/") {
		goal_->SetPos({548.0f, 0.0f, -17.0f});
	} else if (stagePath == "Stage2/") {
		goal_->SetPos({560.0f, 0.0f, 0.0f});
	} else if (stagePath == "Stage3/") {
		goal_->SetPos({601.0f, 0.0f, 0.0f});
	}
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

void Stage::Draw() {
	ground_->Draw();
	doorManager_->Draw();
	wallManager_->Draw();
	barricadeManager_->Draw();
	cageManager_->Draw();
	soapManager_->Draw();
	goal_->Draw();
}

void Stage::DrawTransparent() {
	glassManager_->Draw();
	healAreaManager_->Draw();
}

void Stage::DrawImGui() {
	wallManager_->DrawImGui();
	barricadeManager_->DrawImGui();
	doorManager_->DrawImGui();
	glassManager_->DrawImGui();
	cageManager_->DrawImGui();
	healAreaManager_->DrawImGui();
	soapManager_->DrawImGui();
}

std::vector<AABB> Stage::GetSolidObstacles() const {
	const auto& walls = wallManager_->GetObjects();
	const auto& barricades = barricadeManager_->GetObjects();
	std::vector<AABB> obstacles;
	obstacles.reserve(walls.size() + barricades.size());

	for (const auto& wall : walls) {
		obstacles.push_back(wall->GetCollision());
	}

	for (const auto& barricade : barricades) {
		if (barricade->IsSolid()) {
			obstacles.push_back(barricade->GetCollision());
		}
	}

	return obstacles;
}
