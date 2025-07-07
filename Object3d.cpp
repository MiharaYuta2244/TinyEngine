#include "Object3d.h"
#include "MathOperator.h"
#include "MathUtility.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include <DirectXMath.h>
#include <fstream>
#include <sstream>

using namespace Microsoft::WRL;
using namespace DirectX;

void Object3d::Initialize(Object3dCommon* object3dCommon, TextureManager* textureManager) {
	object3dCommon_ = object3dCommon;
	textureManager_ = textureManager;

	// モデル読み込み
	modelData_ = LoadObjFile("resources", "fence.obj");

	// 頂点データ作成
	CreateVertexData();

	// マテリアルデータ作成
	CreateMaterialData();

	// 座標変換行列データ作成
	CreateTransformationMatrixData();

	// 平行光源データ作成
	CreateDirectionalLightData();

	// CameraForGPU
	CreateCameraForGPUData();

	// フォグパラメータデータ作成
	CreateFogParamData();

	// タイムパラメータデータ作成
	CreateTimeParamData();

	// .objの参照しているテクスチャファイル読み込み
	textureManager_->LoadTexture(modelData_.material.textureFilePath);

	// 読み込んだテクスチャの番号を取得
	modelData_.material.textureIndex = textureManager_->GetTextureIndexByFilePath(modelData_.material.textureFilePath);

	// Transform変数を作る
	transform_ = {
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	cameraTransform_ = {
	    {1.0f, 1.0f, 1.0f  },
        {0.3f, 0.0f, 0.0f  },
        {0.0f, 4.0f, -10.0f}
    };
}

void Object3d::Update() {
	// fogの揺れ
	float elapsedTime = 1.0f / 60.0f;
	static float totalTime = 0.0f;
	totalTime += elapsedTime;

	timeParam_.time = totalTime;

	Matrix4x4 worldMatrix = MathUtility::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 cameraMatrix = MathUtility::MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	Matrix4x4 viewMatrix = MathUtility::Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MathUtility::MakePerspectiveFovMatrix(0.45f, static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight), 0.1f, 100.0f);
	transformMatrixData_->WVP = worldMatrix * viewMatrix * projectionMatrix;
	transformMatrixData_->World = worldMatrix;

	*transformMatrixData_ = {transformMatrixData_->WVP, transformMatrixData_->World};
	*materialData_ = material_;
	*directionalLightData_ = directionalLight_;
	*cameraForGPUData_ = cameraForGPU_;
	*fogParamData_ = fogParam_;
	*timeParamData_ = timeParam_;
}

void Object3d::Draw() {
	object3dCommon_->GetDirectXCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定
	// commandList->IASetIndexBuffer(&indexBufferView_); // IBVを設定
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureManager_->GetSrvHandleGPU(2));
	// マテリアルCBufferの場所を設定
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// wvp用のBufferの場所を設定
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	// ライティングCBufferの場所を指定
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	// CameraForGPUCBufferの場所を指定
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraForGPUResource_->GetGPUVirtualAddress());
	// FogParamCBufferの場所を指定
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, fogParamResource_->GetGPUVirtualAddress());
	// TimeParamCBufferの場所を指定
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, timeParamResource_->GetGPUVirtualAddress());
	// 描画!(DrawCall/ドローコール)。
	object3dCommon_->GetDirectXCommon()->GetCommandList()->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);
}

ModelData Object3d::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;            // 構築するModelData
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals;   // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line;               // ファイルから読んだ1行を格納するもの

	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open());                             // とりあえず開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む

		// identifierに応じた処理
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		} else if (identifier == "f") {
			VertexData triangle[3];
			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してi\Indexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				VertexData vertex = {position, texcoord, normal};
				modelData.vertices.push_back(vertex);
				triangle[faceVertex] = {position, texcoord, normal};
			}
			// 頂点を逆順で登録することで、周り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			// materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}

MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData materialData;                          // 構築するMaterialData
	std::string line;                                   // ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open());                             // とりあえず開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

ComPtr<ID3D12Resource> Object3d::CreateBufferResource(ComPtr<ID3D12Device> device, size_t sizeBytes) {
	if (!device)
		return nullptr;

	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // uploadHeapを使う
	// 頂点にリソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeBytes; // リソースのサイズ。今回はVector4を3頂点分
	// バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

void Object3d::CreateVertexData() {
	// 頂点リソースの作成
	vertexResource_ = CreateBufferResource(object3dCommon_->GetDirectXCommon()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	vertexResource_->Unmap(0, nullptr);
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}

void Object3d::CreateMaterialData() {
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource_ = CreateBufferResource(object3dCommon_->GetDirectXCommon()->GetDevice(), sizeof(Material));
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialResource_->Unmap(0, nullptr);
	// 三角形の色
	material_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	material_.enableLighting = true;
	// uvTransformなどのデータを設定
	material_.uvTransform = MathUtility::MakeIdentity4x4();
	// 反射強度
	material_.shininess = 1.0f;
	// materialDataに代入
	*materialData_ = material_;
}

void Object3d::CreateTransformationMatrixData() {
	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	wvpResource_ = CreateBufferResource(object3dCommon_->GetDirectXCommon()->GetDevice(), sizeof(TransformationMatrix));
	// 書き込むためのアドレスを取得
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformMatrixData_));
	wvpResource_->Unmap(0, nullptr);
	// 単位行列を書き込んでおく
	*transformMatrixData_ = {MathUtility::MakeIdentity4x4(), worldMatrix_};
}

void Object3d::CreateDirectionalLightData() {
	// 平行光源用のリソースを作る
	directionalLightResource_ = CreateBufferResource(object3dCommon_->GetDirectXCommon()->GetDevice(), sizeof(DirectionalLight));
	// アドレス取得
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	directionalLightResource_->Unmap(0, nullptr);
	// 初期化
	directionalLight_.color = {1.0f, 1.0f, 1.0f, 1.0f};
	directionalLight_.direction = {0.0f, -1.0f, 0.0f};
	directionalLight_.intensity = 0.0f;

	// Vector3 → XMVECTOR 変換
	XMVECTOR dirVec = XMVectorSet(
	    directionalLight_.direction.x, directionalLight_.direction.y, directionalLight_.direction.z,
	    0.0f // ← w成分は不要なので0
	);
	// 正規化
	dirVec = XMVector3Normalize(dirVec);
	// XMVECTOR → Vector3 に戻す
	XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&directionalLight_.direction), dirVec);

	// directionalLightDataへの書き込み
	*directionalLightData_ = directionalLight_;
}

void Object3d::CreateCameraForGPUData() {
	// cameraForGPUのリソースを作る
	cameraForGPUResource_ = CreateBufferResource(object3dCommon_->GetDirectXCommon()->GetDevice(), sizeof(CameraForGPU));
	// アドレス取得
	cameraForGPUResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData_));
	cameraForGPUResource_->Unmap(0, nullptr);
	cameraForGPU_ = {0.0f, 0.0f, -10.0f};
	// cameraForGPUDataへの書き込み
	*cameraForGPUData_ = cameraForGPU_;
}

void Object3d::CreateFogParamData() {
	// フォグパラメータのリソースを作る
	fogParamResource_ = CreateBufferResource(object3dCommon_->GetDirectXCommon()->GetDevice(), sizeof(FogParam));
	// アドレス取得
	fogParamResource_->Map(0, nullptr, reinterpret_cast<void**>(&fogParamData_));
	fogParamResource_->Unmap(0, nullptr);
	// 初期化
	fogParam_.fogCenter = {0.0f, 0.0f, 0.0f};
	fogParam_.fogColor = {1.0f, 1.0f, 1.0f};
	fogParam_.fogIntensity = 0.5f;
	fogParam_.radius = 20.0f;
	// fogParamDataへの書き込み
	*fogParamData_ = fogParam_;
}

void Object3d::CreateTimeParamData() {
	// タイムパラメータのリソースを作る
	timeParamResource_ = CreateBufferResource(object3dCommon_->GetDirectXCommon()->GetDevice(), sizeof(TimeParam));
	// アドレス取得
	timeParamResource_->Map(0, nullptr, reinterpret_cast<void**>(&timeParamData_));
	timeParamResource_->Unmap(0, nullptr);
	// 初期化
	timeParam_.time = 1.0f / 60.0f;
	// timeParamDataへの書き込み
	*timeParamData_ = timeParam_;
}
