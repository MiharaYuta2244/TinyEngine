#pragma once
struct Vector3 final
{
	float x;
	float y;
	float z;

	bool operator==(const Vector3& other) const { return x == other.x && y == other.y && z == other.z; }
};