#include "Easing.h"

// --- Sine ---
float Easing::easeInSine(float t) { return 1.0f - cos((t * std::numbers::pi_v<float>) / 2.0f); }
float Easing::easeOutSine(float t) { return sin((t * std::numbers::pi_v<float>) / 2.0f); }
float Easing::easeInOutSine(float t) { return -(cos(std::numbers::pi_v<float> * t) - 1.0f) / 2.0f; }

// --- Quadratic ---
float Easing::easeInQuad(float t) { return t * t; }
float Easing::easeOutQuad(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }
float Easing::easeInOutQuad(float t) { return t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f; }

// --- Cubic ---
float Easing::easeInCubic(float t) { return t * t * t; }
float Easing::easeOutCubic(float t) { return 1.0f - pow(1.0f - t, 3.0f); }
float Easing::easeInOutCubic(float t) { return t < 0.5f ? 4.0f * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f; }

// --- Quartic ---
float Easing::easeInQuart(float t) { return t * t * t * t; }
float Easing::easeOutQuart(float t) { return 1.0f - pow(1.0f - t, 4.0f); }
float Easing::easeInOutQuart(float t) { return t < 0.5f ? 8.0f * pow(t, 4.0f) : 1.0f - pow(-2.0f * t + 2.0f, 4.0f) / 2.0f; }

// --- Quintic ---
float Easing::easeInQuint(float t) { return t * t * t * t * t; }
float Easing::easeOutQuint(float t) { return 1.0f - pow(1.0f - t, 5.0f); }
float Easing::easeInOutQuint(float t) { return t < 0.5f ? 16.0f * pow(t, 5.0f) : 1.0f - pow(-2.0f * t + 2.0f, 5.0f) / 2.0f; }

// --- Expo ---
float Easing::easeInExpo(float t) { return (t == 0.0f) ? 0.0f : pow(2.0f, 10.0f * t - 10.0f); }
float Easing::easeOutExpo(float t) { return (t == 1.0f) ? 1.0f : 1.0f - pow(2.0f, -10.0f * t); }
float Easing::easeInOutExpo(float t) {
	if (t == 0.0f)
		return 0.0f;
	if (t == 1.0f)
		return 1.0f;
	return t < 0.5f ? pow(2.0f, 20.0f * t - 10.0f) / 2.0f : (2.0f - pow(2.0f, -20.0f * t + 10.0f)) / 2.0f;
}

// --- Circ ---
float Easing::easeInCirc(float t) { return 1.0f - sqrt(1.0f - t * t); }
float Easing::easeOutCirc(float t) { return sqrt(1.0f - pow(t - 1.0f, 2.0f)); }
float Easing::easeInOutCirc(float t) { return t < 0.5f ? (1.0f - sqrt(1.0f - pow(2.0f * t, 2.0f))) / 2.0f : (sqrt(1.0f - pow(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f; }

// --- Back ---
float Easing::easeInBack(float t) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;
	return c3 * t * t * t - c1 * t * t;
}
float Easing::easeOutBack(float t) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;
	return 1.0f + c3 * pow(t - 1.0f, 3.0f) + c1 * pow(t - 1.0f, 2.0f);
}
float Easing::easeInOutBack(float t) {
	const float c1 = 1.70158f;
	const float c2 = c1 * 1.525f;
	return t < 0.5f ? (pow(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f : (pow(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
}

// --- Elastic ---
float Easing::easeInElastic(float t) {
	const float c4 = (2.0f * std::numbers::pi_v<float>) / 3.0f;
	return (t == 0.0f) ? 0.0f : (t == 1.0f) ? 1.0f : -pow(2.0f, 10.0f * t - 10.0f) * sin((t * 10.0f - 10.75f) * c4);
}
float Easing::easeOutElastic(float t) {
	const float c4 = (2.0f * std::numbers::pi_v<float>) / 3.0f;
	return (t == 0.0f) ? 0.0f : (t == 1.0f) ? 1.0f : pow(2.0f, -10.0f * t) * sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}
float Easing::easeInOutElastic(float t) {
	const float c5 = (2.0f * std::numbers::pi_v<float>) / 4.5f;
	if (t == 0.0f)
		return 0.0f;
	if (t == 1.0f)
		return 1.0f;
	return t < 0.5f ? -(pow(2.0f, 20.0f * t - 10.0f) * sin((20.0f * t - 11.125f) * c5)) / 2.0f : (pow(2.0f, -20.0f * t + 10.0f) * sin((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
}

// --- Bounce ---
float Easing::easeOutBounce(float t) {
	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (t < 1.0f / d1) {
		return n1 * t * t;
	} else if (t < 2.0f / d1) {
		t -= 1.5f / d1;
		return n1 * t * t + 0.75f;
	} else if (t < 2.5f / d1) {
		t -= 2.25f / d1;
		return n1 * t * t + 0.9375f;
	} else {
		t -= 2.625f / d1;
		return n1 * t * t + 0.984375f;
	}
}

float Easing::easeInBounce(float t) { return 1.0f - easeOutBounce(1.0f - t); }

float Easing::easeInOutBounce(float t) { return t < 0.5f ? (1.0f - easeOutBounce(1.0f - 2.0f * t)) / 2.0f : (1.0f + easeOutBounce(2.0f * t - 1.0f)) / 2.0f; }