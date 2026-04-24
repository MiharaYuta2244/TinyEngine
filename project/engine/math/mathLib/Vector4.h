#pragma once
#include <nlohmann/json.hpp>

using Json = nlohmann::json;

struct Vector4 final {
	float x;
	float y;
	float z;
	float w;
};

// セーブ用 Vector4 -> Jsonへの変換ルール
inline void to_json(Json& j, const Vector4& v) {
	j = Json{
	    {"x", v.x},
        {"y", v.y},
        {"z", v.z},
        {"w", v.w}
    };
}

// ロード用 Json -> Vector4への変換ルール
inline void from_json(const Json& j, Vector4& v) {
	v.x = j.value("x", 0.0f);
	v.y = j.value("y", 0.0f);
	v.z = j.value("z", 0.0f);
	v.w = j.value("w", 0.0f);
}