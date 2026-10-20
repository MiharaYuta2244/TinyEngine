#include "StageMiniature.h"
#include "CameraForGPU.h"
#include "DirectXUtils.h"
#include "FogParam.h"
#include "JsonManager.h"
#include "MathUtility.h"
#include "TimeParam.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include "VertexData.h"
#include <algorithm>
#include <cmath>

struct StageMiniature::MeshBuild {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
};

namespace {
// ミニチュア用の高さ
constexpr float kWallHalfH = 4.0f;
constexpr float kDoorHalfH = 3.5f;
constexpr float kCageHalfH = 3.0f;
constexpr float kGlassHalfH = 3.0f;

// 箱の追加
void AddBox(std::vector<VertexData>& vertices, std::vector<uint32_t>& indices, float px, float py, float pz, const Vector3& h, float yaw, float originX, float originZ) {
	static const Vector3 kNormals[6] = {
	    {1,  0,  0 },
        {-1, 0,  0 },
        {0,  1,  0 },
        {0,  -1, 0 },
        {0,  0,  1 },
        {0,  0,  -1}
    };

	static const float kSigns[4][2] = {
	    {-1, -1},
        {-1, 1 },
        {1,  1 },
        {1,  -1}
    };

	const float c = std::cos(yaw), s = std::sin(yaw);

	for (const Vector3& n : kNormals) {
		Vector3 u = std::abs(n.y) > 0.5f ? Vector3{0, 0, 1} : Vector3{0, 1, 0};
		Vector3 r = MathUtility::Cross(n, u);

		// MakeYawRotateMatrixと同じ向きでY回転
		Vector3 wn = {n.x * c + n.z * s, n.y, -n.x * s + n.z * c};

		uint32_t base = static_cast<uint32_t>(vertices.size());
		for (const auto& sg : kSigns) {
			Vector3 local = {(n.x + sg[0] * r.x + sg[1] * u.x) * h.x, (n.y + sg[0] * r.y + sg[1] * u.y) * h.y, (n.z + sg[0] * r.z + sg[1] * u.z) * h.z};
			VertexData v{};
			v.position = {local.x * c + local.z * s + px - originX, local.y + py, -local.x * s + local.z * c + pz - originZ, 1.0f};
			v.texcoord = {0.0f, 0.0f};
			v.normal = wn;
			vertices.push_back(v);
		}
		indices.insert(indices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
	}
}
} // namespace

void StageMiniature::Initialize(EngineContext* ctx, const std::string& stagePath) {
	ctx_ = ctx;
	layers_.clear();

	// JSONから箱を集める
	std::vector<Box> walls, doors, cages, glasses;

	auto load = [&](const std::string& file, float halfH, bool isDoor, std::vector<Box>& out) {
		std::vector<Json> entries;
		if (!JsonManager::Load(stagePath + file, entries)) {
			return; // ファイルが無ければ何も出さない
		}

		for (const auto& j : entries) {
			Transform t = j.get<Transform>();
			Vector3 half = {t.scale.x, halfH, t.scale.z};
			Vector3 pos = {t.translate.x, halfH, t.translate.z}; // 地面に置く
			out.push_back({pos, half, t.rotate.y});

			if (isDoor) {
				// Doorは2枚構成。2枚目は (0,0,2) をY回転した分だけずらした位置
				float sinY = std::sin(t.rotate.y), cosY = std::cos(t.rotate.y);
				out.push_back({
				    {pos.x - 2.0f * sinY, pos.y, pos.z - 2.0f * cosY},
                    half, t.rotate.y
                });
			}
		}
	};

	load("Walls.json", kWallHalfH, false, walls);
	load("Doors.json", kDoorHalfH, true, doors);
	load("Cages.json", kCageHalfH, false, cages);
	load("Glasses.json", kGlassHalfH, false, glasses);

	// 全体のバウンディングからXZ中心を求める
	float minX = 1e9f, maxX = -1e9f, minZ = 1e9f, maxZ = -1e9f;
	for (auto* list : {&walls, &doors, &cages, &glasses}) {
		for (const Box& b : *list) {
			float e = std::max(b.half.x, b.half.z);
			minX = std::min(minX, b.pos.x - e);
			maxX = std::max(maxX, b.pos.x + e);
			minZ = std::min(minZ, b.pos.z - e);
			maxZ = std::max(maxZ, b.pos.z + e);
		}
	}

	if (minX > maxX) {
		return; // 何も無いステージ
	}

	const float cx = (minX + maxX) * 0.5f, cz = (minZ + maxZ) * 0.5f;

	// 床 + 種類ごとのメッシュを作る
	{
		MeshBuild m;
		const float margin = 5.0f;
		AddBox(m.vertices, m.indices, cx, -1.0f, cz, {(maxX - minX) * 0.5f + margin, 1.0f, (maxZ - minZ) * 0.5f + margin}, 0.0f, cx, cz);
		CreateLayer(m, {0.12f, 0.14f, 0.2f, 1.0f}, false);
	}

	auto build = [&](const std::vector<Box>& list, const Vector4& color, bool transparent) {
		if (list.empty()) {
			return;
		}

		MeshBuild m;
		for (const Box& b : list) {
			AddBox(m.vertices, m.indices, b.pos.x, b.pos.y, b.pos.z, b.half, b.yaw, cx, cz);
		}

		CreateLayer(m, color, transparent);
	};

	build(walls, {0.75f, 0.8f, 0.95f, 1.0f}, false);
	build(doors, {0.9f, 0.6f, 0.2f, 1.0f}, false);
	build(cages, {0.4f, 0.4f, 0.45f, 1.0f}, false);
	build(glasses, {0.5f, 0.9f, 1.0f, 0.35f}, true);

	CreateConstantBuffers();
}

void StageMiniature::CreateLayer(const MeshBuild& mesh, const Vector4& color, bool transparent) {
	auto device = ctx_->object3dCommon->GetDxCommon()->GetDevice();
	Layer L;
	L.color = color;
	L.transparent = transparent;
	L.indexCount = static_cast<uint32_t>(mesh.indices.size());

	// 頂点バッファ
	UINT vbSize = static_cast<UINT>(sizeof(VertexData) * mesh.vertices.size());
	L.vertexRes = DirectXUtils::CreateBufferResource(device, vbSize);
	void* p = nullptr;
	L.vertexRes->Map(0, nullptr, &p);
	std::memcpy(p, mesh.vertices.data(), vbSize);
	L.vertexRes->Unmap(0, nullptr);
	L.vbv = {L.vertexRes->GetGPUVirtualAddress(), vbSize, sizeof(VertexData)};

	// インデックスバッファ
	UINT ibSize = static_cast<UINT>(sizeof(uint32_t) * mesh.indices.size());
	L.indexRes = DirectXUtils::CreateBufferResource(device, ibSize);
	L.indexRes->Map(0, nullptr, &p);
	std::memcpy(p, mesh.indices.data(), ibSize);
	L.indexRes->Unmap(0, nullptr);
	L.ibv = {L.indexRes->GetGPUVirtualAddress(), ibSize, DXGI_FORMAT_R32_UINT};

	// マテリアル(レイヤーごとに色が違うので別バッファ)
	L.materialRes = DirectXUtils::CreateBufferResource(device, sizeof(Material));
	L.materialRes->Map(0, nullptr, reinterpret_cast<void**>(&L.materialData));
	Material m{};
	m.color = color;
	m.uvTransform = MathUtility::MakeIdentity4x4();
	m.enableLighting = true;
	m.shininess = 16.0f;
	*L.materialData = m;

	layers_.push_back(std::move(L));
}

void StageMiniature::CreateConstantBuffers() {
	auto device = ctx_->object3dCommon->GetDxCommon()->GetDevice();

	wvpRes_ = DirectXUtils::CreateBufferResource(device, sizeof(TransformationMatrix));
	wvpRes_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));

	cameraRes_ = DirectXUtils::CreateBufferResource(device, sizeof(CameraForGPU));
	cameraRes_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));

	// フォグと時間はObject3dと同じルートパラメータを埋めるために用意
	FogParam* fog = nullptr;
	fogRes_ = DirectXUtils::CreateBufferResource(device, sizeof(FogParam));
	fogRes_->Map(0, nullptr, reinterpret_cast<void**>(&fog));
	*fog = {
	    {0, 0, 0},
        1.0f, {1, 1, 1},
        0.0f
    };

	TimeParam* time = nullptr;
	timeRes_ = DirectXUtils::CreateBufferResource(device, sizeof(TimeParam));
	timeRes_->Map(0, nullptr, reinterpret_cast<void**>(&time));
	time->time = 0.0f;
}

void StageMiniature::Update(const Vector3& center, float yaw, float scale, float brightness) {
	if (layers_.empty()) {
		return;
	}

	Matrix4x4 world = MathUtility::MakeAffineMatrix({scale, scale, scale}, Vector3{0.0f, yaw, 0.0f}, center);
	Camera* camera = ctx_->object3dCommon->GetDefaultCamera();

	if (camera) {
		cameraData_->worldPosition = camera->GetTranslation();
		wvpData_->WVP = MathUtility::Multiply(world, camera->GetViewProjectionMatrix());
	} else {
		wvpData_->WVP = world;
	}

	wvpData_->World = world;
	wvpData_->WorldInverseTranspose = MathUtility::MakeAffineMatrix({1, 1, 1}, Vector3{0.0f, yaw, 0.0f}, {0, 0, 0}); // 法線用は回転のみ(縮尺は含めない)

	for (Layer& L : layers_) {
		L.materialData->color = {L.color.x * brightness, L.color.y * brightness, L.color.z * brightness, L.color.w};
	}
}

void StageMiniature::Draw() { DrawLayers(false); }

void StageMiniature::DrawTransparent() { DrawLayers(true); }

void StageMiniature::DrawLayers(bool transparent) {
	bool any = std::any_of(layers_.begin(), layers_.end(), [&](const Layer& l) { return l.transparent == transparent; });
	if (!any) {
		return;
	}

	auto* common = ctx_->object3dCommon;
	auto cmd = common->GetDxCommon()->GetCommandList();
	if (transparent) {
		common->DrawSettingTransparent(ctx_->textureManager);
	} else {
		common->DrawSettingCommon(ctx_->textureManager);
	}

	for (const Layer& L : layers_) {
		if (L.transparent != transparent) {
			continue;
		}

		cmd->SetGraphicsRootDescriptorTable(2, ctx_->textureManager->GetSrvHandleGPU("resources/textures/white.png"));
		cmd->SetGraphicsRootConstantBufferView(0, L.materialRes->GetGPUVirtualAddress());
		cmd->SetGraphicsRootConstantBufferView(1, wvpRes_->GetGPUVirtualAddress());
		cmd->SetGraphicsRootConstantBufferView(4, cameraRes_->GetGPUVirtualAddress());
		cmd->SetGraphicsRootConstantBufferView(5, fogRes_->GetGPUVirtualAddress());
		cmd->SetGraphicsRootConstantBufferView(6, timeRes_->GetGPUVirtualAddress());
		cmd->IASetVertexBuffers(0, 1, &L.vbv);
		cmd->IASetIndexBuffer(&L.ibv);
		cmd->DrawIndexedInstanced(L.indexCount, 1, 0, 0, 0);
	}
}