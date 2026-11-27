#pragma once
#include "EngineContext.h"
#include "Material.h"
#include "Vector3.h"
#include "Vector4.h"
#include <d3d12.h>
#include <string>
#include <vector>
#include <wrl.h>

/// <summary>
/// ステージのJSONから壁などを箱として結合し、立体のミニチュアを描画する
/// </summary>
class StageMiniature {
public:
	// 初期化処理
	void Initialize(EngineContext* ctx, const std::string& stagePath);

	// 更新処理
	void Update(const Vector3& center, float yaw, float scale, float brightness);

	// 不透明オブジェクト描画処理
	void Draw();

	// 半透明オブジェクト描画処理
	void DrawTransparent();

private:
	struct Box {
		Vector3 pos;
		Vector3 half;
		float yaw;
	};

	struct Layer {
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexRes, indexRes, materialRes;
		D3D12_VERTEX_BUFFER_VIEW vbv{};
		D3D12_INDEX_BUFFER_VIEW ibv{};
		uint32_t indexCount = 0;
		Material* materialData = nullptr;
		Vector4 color{};
		bool transparent = false;
	};

	struct MeshBuild;

	void CreateLayer(const MeshBuild& mesh, const Vector4& color, bool transparent);
	void CreateConstantBuffers();
	void DrawLayers(bool transparent);

private:
	EngineContext* ctx_ = nullptr;
	std::vector<Layer> layers_;

	// 全レイヤー共通
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpRes_, cameraRes_, fogRes_, timeRes_;
	struct TransformationMatrix* wvpData_ = nullptr;
	struct CameraForGPU* cameraData_ = nullptr;
};