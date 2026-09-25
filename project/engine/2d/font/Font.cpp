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

void Font::BuildAtlas(const std::wstring& fontFamily, int pixelHeight, const std::wstring& chars) {
	// グリッド配置
	int cellW = pixelHeight;
	int cellH = pixelHeight + pixelHeight / 4;
	int columns = static_cast<int>(std::ceil(std::sqrt((double)chars.size())));
	int rows = static_cast<int>(std::ceil((double)chars.size() / columns));
	uint32_t atlasW = columns * cellW;
	uint32_t atlasH = rows * cellH;

	HDC screenDC = GetDC(nullptr);
	HDC memDC = CreateCompatibleDC(screenDC);

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
	memset(bits, 0, atlasW * atlasH * 4);

	HFONT font = CreateFontW(
	    -pixelHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontFamily.c_str());
	HGDIOBJ oldFont = SelectObject(memDC, font);
	SetTextColor(memDC, RGB(255, 255, 255));
	SetBkMode(memDC, TRANSPARENT);

	TEXTMETRICW tm{};
	GetTextMetricsW(memDC, &tm);
	lineHeight_ = static_cast<float>(tm.tmHeight);

	int col = 0, row = 0;
	for (wchar_t c : chars) {
		SIZE sz{};
		GetTextExtentPoint32W(memDC, &c, 1, &sz);

		int cx = col * cellW;
		int cy = row * cellH;
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

	texturePath_ = "font/" + StringUtility::ConvertString(fontFamily) + "_" + std::to_string(pixelHeight);
	ctx_->textureManager->RegisterTextureFromMemory(texturePath_, rgba.data(), atlasW, atlasH);
}