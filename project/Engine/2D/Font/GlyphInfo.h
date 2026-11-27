#pragma once
#include "Vector2.h"

struct GlyphInfo {
	Vector2 uvMin{};      // アトラス内UV(左上)
	Vector2 uvMax{};      // アトラス内UV(右下)
	Vector2 size{};       // グリフのピクセルサイズ
	Vector2 bearing{};    // ペン位置からの描画オフセット
	float advance = 0.0f; // 次の文字へ進む量(px)
};