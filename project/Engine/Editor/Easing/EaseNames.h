#pragma once
#include "EaseType.h"
#include <iterator>

/// <summary>
/// エディタ用の名前表
/// </summary>
inline constexpr const char* kEaseNames[] = {
    "EaseInSine",     "EaseOutSine",   "EaseInOutSine", "EaseInQuad",     "EaseOutQuad",      "EaseInOutQuad", "EaseInCubic",   "EaseOutCubic",    "EaseInOutCubic", "EaseInQuart",   "EaseOutQuart",
    "EaseInOutQuart", "EaseInQuint",   "EaseOutQuint",  "EaseInOutQuint", "EaseInExpo",       "EaseOutExpo",   "EaseInOutExpo", "EaseInCirc",      "EaseOutCirc",    "EaseInOutCirc", "EaseInBack",
    "EaseOutBack",    "EaseInOutBack", "EaseInElastic", "EaseOutElastic", "EaseInOutElastic", "EaseOutBounce", "EaseInBounce",  "EaseInOutBounce", "Linear",
};

static_assert(std::size(kEaseNames) == static_cast<size_t>(EaseType::EASELINEAR) + 1, "kEaseNamesとEaseTypeの数が一致していません");