#pragma once
#include "EasingAnimation.h"

/// <summary>
/// アニメーションのバンドルを管理する構造体
/// </summary>
/// <typeparam name="T"></typeparam>
template<typename T> struct AnimationBundle {
	EasingAnimation<T> anim; // アニメーション本体
	T start;                 // 開始値
	T end;                   // 目標値
	T temp;                  // 一時値
};
