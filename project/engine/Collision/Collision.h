#pragma once
#include "AABB.h"
#include "OBB.h"
#include "Sphere.h"

struct Segment {
	Vector3 origin; // 始点
	Vector3 diff;   // 始点から終点へのベクトル
};

struct Ray {
	Vector3 origin;    // レイの開始位置
	Vector3 direction; // レイの方向
};

namespace Collision {

// 未対応の型の組み合わせをコンパイル時に検出する。
template<typename T1, typename T2> bool Intersect(const T1& a, const T2& b) {
	static_assert(sizeof(T1) == 0, "IsCollision not implemented for these types.");
	return false;
}

// 球同士の交差を判定する
bool Intersect(const Sphere& s1, const Sphere& s2);

// AABB 同士の交差を判定する
bool Intersect(const AABB& aabb1, const AABB& aabb2);

// Y軸を除き、XZ平面上でAABB同士の交差を判定する。
bool IntersectXZ(const AABB& a, const AABB& b);

// めり込んだAABBをXZ平面上で押し戻す最小の変位を返す
Vector3 CalculatePushbackXZ(const AABB& moving, const AABB& obstacle);

// AABBと球の交差を判定する
bool Intersect(const AABB& aabb, const Sphere& sphere);

// 点がAABBの内側または境界上にあるか判定する
bool Intersect(const AABB& aabb, const Vector3& point);

// OBB同士の交差を判定する
bool Intersect(const OBB& obb1, const OBB& obb2);

// AABBとOBBの交差を判定する
bool Intersect(const AABB& aabb, const OBB& obb);

// OBBとAABBの交差を判定する
bool Intersect(const OBB& obb, const AABB& aabb);

// 線分とAABBの交差を判定し、最初の交差位置を線分上の割合でoutTに返す
bool Intersect(const Segment& segment, const AABB& aabb, float& outT);

// レイとAABBの交差を判定し、最初の交差位置のレイパラメータをoutDistanceに返す
bool Intersect(const Ray& ray, const AABB& aabb, float& outDistance);
}; // namespace Collision
