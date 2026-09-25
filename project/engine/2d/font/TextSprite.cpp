#include "TextSprite.h"

using namespace TinyEngine;

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