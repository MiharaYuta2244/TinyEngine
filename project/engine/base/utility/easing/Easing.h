#pragma once
#include <cmath>
#include <numbers>

class Easing {
public:
	// Sine
	static float easeInSine(float t);
	static float easeOutSine(float t);
	static float easeInOutSine(float t);

	// Quadratic
	static float easeInQuad(float t);
	static float easeOutQuad(float t);
	static float easeInOutQuad(float t);

	// Cubic
	static float easeInCubic(float t);
	static float easeOutCubic(float t);
	static float easeInOutCubic(float t);

	// Quartic
	static float easeInQuart(float t);
	static float easeOutQuart(float t);
	static float easeInOutQuart(float t);

	// Quintic
	static float easeInQuint(float t);
	static float easeOutQuint(float t);
	static float easeInOutQuint(float t);

	// Expo
	static float easeInExpo(float t);
	static float easeOutExpo(float t);
	static float easeInOutExpo(float t);

	// Circ
	static float easeInCirc(float t);
	static float easeOutCirc(float t);
	static float easeInOutCirc(float t);

	// Back
	static float easeInBack(float t);
	static float easeOutBack(float t);
	static float easeInOutBack(float t);

	// Elastic
	static float easeInElastic(float t);
	static float easeOutElastic(float t);
	static float easeInOutElastic(float t);

	// Bounce
	static float easeOutBounce(float t);
	static float easeInBounce(float t);
	static float easeInOutBounce(float t);
};