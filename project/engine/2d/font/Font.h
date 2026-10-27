#pragma once
#include "EngineContext.h"
#include "GlyphInfo.h"
#include <string>
#include <unordered_map>

namespace TinyEngine {
class Font {
public:
	// 初期化関数
	void Initialize(EngineContext* ctx, const std::wstring& fontFamily, int pixelHeight, const std::wstring& charSet = L"");
	~Font() = default;

	// フォントファイルの登録解除
	static void UnloadFontFiles();

	const GlyphInfo* GetGlyph(wchar_t c) const;
	float GetLineHeight() const { return lineHeight_; }
	const std::string& GetTexturePath() const { return texturePath_; }

	// TTFファイルをプライベート登録する
	static bool LoadFontFile(const std::wstring& ttfPath);

	// 日本語表示に必要な文字セットを作る(ASCII + 記号 + ひらがな + カタカナ + 全角英数)
	static std::wstring MakeJapaneseCharSet(bool jisLevel1Kanji = false);

private:
	void BuildAtlas(const std::wstring& fontFamily, int pixelHeight, const std::wstring& charsIn);

private:
	EngineContext* ctx_ = nullptr;
	std::unordered_map<wchar_t, GlyphInfo> glyphs_;
	std::string texturePath_;
	float lineHeight_ = 0.0f;

	// 解放管理用
	static inline std::vector<std::wstring> loadedFontPaths_;
};
} // namespace TinyEngine