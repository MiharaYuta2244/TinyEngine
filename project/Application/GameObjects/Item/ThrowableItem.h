#pragma once
#include "GameObjects/PlaceableObject.h"

class ThrowableItem : public PlaceableObject {
public:
	virtual bool CanGrab() const = 0;
	virtual void Grab() = 0;
	virtual void Throw(const Vector3& direction) = 0;

	Vector3 GetPos() const { return transform_.translate; }
	void SetPos(Vector3 pos) { transform_.translate = pos; }
};
