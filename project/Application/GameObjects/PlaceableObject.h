#pragma once
#include "IGameObject.h"
#include "EngineContext.h"

/// <summary>
/// ステージオブジェクト専用中間クラス
/// </summary>
class PlaceableObject : public IGameObject {
public:
	virtual void Draw() = 0;

	// Transform以外の固有データをJSONへ書き出す
	virtual void ToExtraJson(Json& j) const {}

	// Transform以外の固有データをJSONから読み込む
	virtual void FromExtraJson(const Json& j) {}
};