#pragma once
#include "EngineContext.h"
#include "Font.h"

namespace TinyEngine {
class TextSprite {
public:
	void Initialize(EngineContext* ctx, Font* font, const std::wstring& text);
	void Update();
	void Draw();

	void SetText(const std::wstring& text);
	void SetPosition(const Vector2& pos) { position_ = pos; }
	void SetColor(const Vector4& color) { color_ = color; }
	void SetScale(float scale) { scale_ = scale; }
	void SetAnchorPoint(const Vector2& anchor) { anchorPoint_ = anchor; }
	Vector2 GetTextSize() const { return textSize_; }

private:
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
	};

	void BuildMesh();
	void EnsureCapacity(size_t charCount);

	EngineContext* ctx_ = nullptr;
	Font* font_ = nullptr;
	std::wstring text_;
	Vector2 position_{}, anchorPoint_{};
	float scale_ = 1.0f;
	Vector4 color_{1, 1, 1, 1};
	Vector2 textSize_{};

	size_t capacityChars_ = 0;
	uint32_t visibleQuadCount_ = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_, indexResource_, materialResource_, transformResource_;
	D3D12_VERTEX_BUFFER_VIEW vbv_{};
	D3D12_INDEX_BUFFER_VIEW ibv_{};
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	Vector4* materialData_ = nullptr;
	Matrix4x4* transformData_ = nullptr;
};
} // namespace TinyEngine