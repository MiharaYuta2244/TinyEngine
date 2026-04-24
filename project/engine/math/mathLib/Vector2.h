#pragma once
#include <nlohmann/json.hpp>

using Json = nlohmann::json;

struct Vector2 final
{
	float x;
	float y;
};

// セーブ用 Vector2 -> Jsonへの変換ルール
inline void to_json(Json& j, const Vector2& v) {
	j = Json{{"x", v.x}, {"y", v.y}};
}

// ロード用 Json -> Vector2への変換ルール
inline void from_json(const Json& j, Vector2& v){
	v.x = j.value("x", 0.0f);
	v.y = j.value("y", 0.0f);
}