#pragma once
#include "Vector3.h"
#include <nlohmann/json.hpp>

using Json = nlohmann::json;

/// <summary>
/// 変換情報構造体
/// </summary>
struct Transform final {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

inline void to_json(Json& j, const Transform& t) {
	j = Json{
	    {"translate", t.translate},
        {"rotate",    t.rotate   },
        {"scale",     t.scale    }
    };
}

inline void from_json(const Json& j, Transform& t) {
	if (j.contains("translate"))
		t.translate = j.at("translate").get<Vector3>();
	if (j.contains("rotate"))
		t.rotate = j.at("rotate").get<Vector3>();
	if (j.contains("scale"))
		t.scale = j.at("scale").get<Vector3>();
}