#pragma once
#include "EngineContext.h"
#include "GlyphInfo.h"
#include <string>
#include <unordered_map>

namespace TinyEngine {
class Font {
public:
	void Initialize(EngineContext* ctx, const std::wstring& fontFamily, int pixelHeight, const std::wstring& charSet = L"");
	~Font();

	const GlyphInfo* GetGlyph(wchar_t c) const;
	float GetLineHeight() const { return lineHeight_; }
	const std::string& GetTexturePath() const { return texturePath_; }

	// TTFファイルをプライベート登録する
	static bool LoadFontFile(const std::wstring& ttfPath);

private:
	void BuildAtlas(const std::wstring& fontFamily, int pixelHeight, const std::wstring& charSet);

private:
	EngineContext* ctx_ = nullptr;
	std::unordered_map<wchar_t, GlyphInfo> glyphs_;
	std::string texturePath_;
	float lineHeight_ = 0.0f;

	// 解放管理用
	static inline std::vector<std::wstring> loadedFontPaths_;
};
} // namespace TinyEngine