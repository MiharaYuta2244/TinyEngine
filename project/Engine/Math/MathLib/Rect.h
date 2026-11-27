#pragma once
#include "Vector2.h"

template <typename T>
struct Rect{
	T right;
	T left;
	T front;
	T back;

	bool CheckOver(Vector2 pos)const {
		if(pos.x > right || pos.x < left || pos.y > front || pos.y < back){
			return true;
		}
		return false;
	}
};