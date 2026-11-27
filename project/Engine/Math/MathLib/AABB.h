#pragma once
#include "Vector3.h"
struct AABB {
	Vector3 min; // 最小点
	Vector3 max; // 最大点

	// 中心とXZ平面上の半幅から、高さのないAABBを作る
	static constexpr AABB FromCenterAndHalfExtentsXZ(const Vector3& center, const Vector3& halfExtents) {
		return {
		    {center.x - halfExtents.x, center.y, center.z - halfExtents.z},
		    {center.x + halfExtents.x, center.y, center.z + halfExtents.z},
		};
	}
};