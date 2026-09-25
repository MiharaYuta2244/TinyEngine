#pragma once
#include "EngineContext.h"
#include "GlyphInfo.h"
#include <string>
#include <unordered_map>

namespace TinyEngine {
class Font {
public:
	void Initialize(EngineContext* ctx, const std::wstring& fontFamily, int pixelHeight, const std::wstring& charSet = L"");

	const GlyphInfo* GetGlyph(wchar_t c) const;
	float GetLineHeight() const { return lineHeight_; }
	const std::string& GetTexturePath() const { return texturePath_; }

private:
	void BuildAtlas(const std::wstring& fontFamily, int pixelHeight, const std::wstring& charSet);

	EngineContext* ctx_ = nullptr;
	std::unordered_map<wchar_t, GlyphInfo> glyphs_;
	std::string texturePath_;
	float lineHeight_ = 0.0f;
};
} // namespace TinyEngine