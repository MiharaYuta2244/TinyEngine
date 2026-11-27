#include "Font.h"
#include "StringUtility.h"

using namespace TinyEngine;

void Font::Initialize(EngineContext* ctx, const std::wstring& fontFamily, int pixelHeight, const std::wstring& charSet) {
	ctx_ = ctx;
	std::wstring chars = charSet;
	if (chars.empty()) {
		for (wchar_t c = 0x20; c <= 0x7E; ++c)
			chars.push_back(c);
	}
	BuildAtlas(fontFamily, pixelHeight, chars);
}

void Font::BuildAtlas(const std::wstring& fontFamily, int pixelHeight, const std::wstring& charsIn) {
	// 重複除去(同じ文字セットなら必ず同じレイアウトになるようソートもする)
	std::wstring chars = charsIn;
	std::sort(chars.begin(), chars.end());
	chars.erase(std::unique(chars.begin(), chars.end()), chars.end());

	HDC screenDC = GetDC(nullptr);
	HDC memDC = CreateCompatibleDC(screenDC);

	HFONT font = CreateFontW(
	    -pixelHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontFamily.c_str());
	HGDIOBJ oldFont = SelectObject(memDC, font);

	// 先にフォントのメトリクスを取ってセルサイズを決める
	TEXTMETRICW tm{};
	GetTextMetricsW(memDC, &tm);
	lineHeight_ = static_cast<float>(tm.tmHeight);

	const int pad = 2; // 隣のグリフへの滲み防止
	int cellW = pixelHeight + pad * 2;
	int cellH = tm.tmHeight + pad * 2;
	int columns = static_cast<int>(std::ceil(std::sqrt((double)chars.size() * cellH / cellW))); // 正方形に近づける
	columns = std::max(columns, 1);
	int rows = static_cast<int>(std::ceil((double)chars.size() / columns));
	uint32_t atlasW = columns * cellW;
	uint32_t atlasH = rows * cellH;
	assert(atlasW <= 16384 && atlasH <= 16384 && "フォントアトラスが大きすぎます。文字数かサイズを減らしてください");

	BITMAPINFO bmi{};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = atlasW;
	bmi.bmiHeader.biHeight = -static_cast<int>(atlasH);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* bits = nullptr;
	HBITMAP bmp = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
	HGDIOBJ oldBmp = SelectObject(memDC, bmp);
	memset(bits, 0, size_t(atlasW) * atlasH * 4);

	SetTextColor(memDC, RGB(255, 255, 255));
	SetBkMode(memDC, TRANSPARENT);

	int col = 0, row = 0;
	for (wchar_t c : chars) {
		SIZE sz{};
		GetTextExtentPoint32W(memDC, &c, 1, &sz);

		int cx = col * cellW + pad;
		int cy = row * cellH + pad;
		TextOutW(memDC, cx, cy, &c, 1);

		GlyphInfo g;
		g.uvMin = {float(cx) / atlasW, float(cy) / atlasH};
		g.uvMax = {float(cx + sz.cx) / atlasW, float(cy + tm.tmHeight) / atlasH};
		g.size = {float(sz.cx), float(tm.tmHeight)};
		g.bearing = {0.0f, 0.0f};
		g.advance = float(sz.cx);
		glyphs_[c] = g;

		if (++col >= columns) {
			col = 0;
			++row;
		}
	}

	auto* src = static_cast<uint8_t*>(bits);
	std::vector<uint8_t> rgba(atlasW * atlasH * 4);
	for (uint32_t i = 0; i < atlasW * atlasH; ++i) {
		uint8_t b = src[i * 4 + 0];
		uint8_t g = src[i * 4 + 1];
		uint8_t r = src[i * 4 + 2];
		uint8_t alpha = static_cast<uint8_t>((r + g + b) / 3);
		rgba[i * 4 + 0] = 255;
		rgba[i * 4 + 1] = 255;
		rgba[i * 4 + 2] = 255;
		rgba[i * 4 + 3] = alpha;
	}

	SelectObject(memDC, oldFont);
	DeleteObject(font);
	SelectObject(memDC, oldBmp);
	DeleteObject(bmp);
	DeleteDC(memDC);
	ReleaseDC(nullptr, screenDC);

	// テクスチャキーに文字セットのハッシュを含める(文字セット違いでの衝突防止)
	size_t h = std::hash<std::wstring>{}(chars);
	texturePath_ = "font/" + StringUtility::ConvertString(fontFamily) + "_" + std::to_string(pixelHeight) + "_" + std::to_string(h);
	ctx_->textureManager->RegisterTextureFromMemory(texturePath_, rgba.data(), atlasW, atlasH);
}

const GlyphInfo* Font::GetGlyph(wchar_t c) const {
	auto it = glyphs_.find(c);
	if (it != glyphs_.end()) {
		return &it->second;
	}
	return nullptr;
}

bool Font::LoadFontFile(const std::wstring& ttfPath) {
	int added = AddFontResourceExW(ttfPath.c_str(), FR_PRIVATE | FR_NOT_ENUM, nullptr);
	if (added == 0) {
		Logger::Log("Failed to load font file: " + StringUtility::ConvertString(ttfPath), LogLevel::Error);
		return false;
	}
	loadedFontPaths_.push_back(ttfPath);
	return true;
}

std::wstring Font::MakeJapaneseCharSet(bool jisLevel1Kanji) {
	std::wstring s;
	auto addRange = [&s](wchar_t from, wchar_t to) {
		for (wchar_t c = from; c <= to; ++c) {
			s.push_back(c);
		}
	};
	addRange(0x20, 0x7E);     // ASCII
	addRange(0x3000, 0x303F); // 全角記号
	addRange(0x3041, 0x3096); // ひらがな
	addRange(0x309B, 0x309F); // 補助記号
	addRange(0x30A0, 0x30FF); // カタカナ
	addRange(0xFF01, 0xFF5E); // 全角英数
	addRange(0xFF61, 0xFF9F); // 半角カナ

	if (jisLevel1Kanji) {
		for (int lead = 0x88; lead <= 0x98; ++lead) {
			for (int trail = 0x40; trail <= 0xFC; ++trail) {
				if (trail == 0x7F)
					continue;
				int code = (lead << 8) | trail;
				if (code < 0x889F || code > 0x9872)
					continue;
				char mb[2] = {static_cast<char>(lead), static_cast<char>(trail)};
				wchar_t wc = 0;
				if (MultiByteToWideChar(932, MB_ERR_INVALID_CHARS, mb, 2, &wc, 1) == 1) {
					s.push_back(wc);
				}
			}
		}
	}
	return s;
}

void Font::UnloadFontFiles() {
	for (auto& path : loadedFontPaths_) {
		RemoveFontResourceExW(path.c_str(), FR_PRIVATE | FR_NOT_ENUM, nullptr);
	}

	loadedFontPaths_.clear();
}