#include "TextSprite.h"
#include "DirectXUtils.h"
#include "MathUtility.h"
#include "WinApp.h"
#include <algorithm>

using namespace TinyEngine;

void TextSprite::Initialize(EngineContext* ctx, Font* font, const std::wstring& text) {
	ctx_ = ctx;
	font_ = font;
	text_ = text;

	auto device = ctx_->fontCommon->GetDirectXCommon()->GetDevice();

	// マテリアル用リソースの作成
	materialResource_ = DirectXUtils::CreateBufferResource(device, sizeof(Vector4));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	*materialData_ = color_;

	// 座標変換行列用リソースの作成
	transformResource_ = DirectXUtils::CreateBufferResource(device, sizeof(Matrix4x4));
	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));
	*transformData_ = MathUtility::MakeIdentity4x4();

	// メッシュ構築
	BuildMesh();
}

void TextSprite::EnsureCapacity(size_t charCount) {
	// 既に十分な容量があれば何もしない
	if (vertexResource_ && charCount <= capacityChars_) {
		return;
	}

	// 最低でも1文字分は確保しておく
	capacityChars_ = std::max<size_t>(charCount, 1);

	auto device = ctx_->fontCommon->GetDirectXCommon()->GetDevice();

	// 頂点バッファ
	size_t vertexCount = capacityChars_ * 4;
	vertexResource_ = DirectXUtils::CreateBufferResource(device, sizeof(VertexData) * vertexCount);
	vbv_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vbv_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertexCount);
	vbv_.StrideInBytes = sizeof(VertexData);
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	// インデックスバッファ
	size_t indexCount = capacityChars_ * 6;
	indexResource_ = DirectXUtils::CreateBufferResource(device, sizeof(uint32_t) * indexCount);
	ibv_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	ibv_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indexCount);
	ibv_.Format = DXGI_FORMAT_R32_UINT;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
}

void TextSprite::SetText(const std::wstring& text) {
	if (text_ == text) {
		return;
	}
	text_ = text;

	// 文字列が変わったのでメッシュを再構築
	BuildMesh();
}

void TextSprite::Update() {
	if (!transformData_) {
		return;
	}

	// アンカーポイント分のオフセット
	Vector2 offset = {-textSize_.x * anchorPoint_.x, -textSize_.y * anchorPoint_.y};

	Matrix4x4 scaleMatrix = MathUtility::MakeScaleMatrix({scale_, scale_, 1.0f});
	Matrix4x4 translateMatrix = MathUtility::MakeTranslateMatrix({position_.x + offset.x * scale_, position_.y + offset.y * scale_, 0.0f});
	Matrix4x4 worldMatrix = MathUtility::Multiply(scaleMatrix, translateMatrix);

	// スプライトと同様に正射影で画面座標系に変換する
	Matrix4x4 viewMatrix = MathUtility::MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MathUtility::MakeOrthographicMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kClientWidth), static_cast<float>(WinApp::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = MathUtility::Multiply(worldMatrix, MathUtility::Multiply(viewMatrix, projectionMatrix));

	*transformData_ = worldViewProjectionMatrix;

	if (materialData_) {
		*materialData_ = color_;
	}
}

void TextSprite::Draw() {
	if (visibleQuadCount_ == 0) {
		return;
	}

	// 共通描画設定
	ctx_->fontCommon->DrawSettingCommon();

	auto commandList = ctx_->fontCommon->GetDirectXCommon()->GetCommandList();
	commandList->IASetVertexBuffers(0, 1, &vbv_);
	commandList->IASetIndexBuffer(&ibv_);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, transformResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, ctx_->textureManager->GetSrvHandleGPU(font_->GetTexturePath()));

	// 実際に文字が描かれている分だけ描画する
	commandList->DrawIndexedInstanced(visibleQuadCount_ * 6, 1, 0, 0, 0);
}

void TextSprite::BuildMesh() {
	EnsureCapacity(text_.size());
	float penX = 0.0f, penY = 0.0f, maxWidth = 0.0f;
	uint32_t q = 0;

	for (wchar_t c : text_) {
		if (c == L'\n') {
			penX = 0.0f;
			penY += font_->GetLineHeight();
			continue;
		}
		const GlyphInfo* g = font_->GetGlyph(c);
		if (!g)
			continue;

		float x0 = penX + g->bearing.x, y0 = penY + g->bearing.y;
		float x1 = x0 + g->size.x, y1 = y0 + g->size.y;

		VertexData* v = &vertexData_[q * 4];
		v[0] = {
		    {x0, y1, 0, 1},
            {g->uvMin.x, g->uvMax.y}
        };
		v[1] = {
		    {x0, y0, 0, 1},
            {g->uvMin.x, g->uvMin.y}
        };
		v[2] = {
		    {x1, y1, 0, 1},
            {g->uvMax.x, g->uvMax.y}
        };
		v[3] = {
		    {x1, y0, 0, 1},
            {g->uvMax.x, g->uvMin.y}
        };

		uint32_t* idx = &indexData_[q * 6];
		uint32_t b = q * 4;
		idx[0] = b;
		idx[1] = b + 1;
		idx[2] = b + 2;
		idx[3] = b + 1;
		idx[4] = b + 3;
		idx[5] = b + 2;

		penX += g->advance;
		maxWidth = std::max(maxWidth, penX);
		++q;
	}
	visibleQuadCount_ = q;
	textSize_ = {maxWidth, penY + font_->GetLineHeight()};
}