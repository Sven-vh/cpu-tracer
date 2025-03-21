#pragma once
#include "Ease.h"

#define SVEN_TWEEN_PI 3.14159265358979323846

template<typename T>
inline T lerp(const T a, const T b, const float t) {
	return a + (b - a) * t;
}

//Source: The formulas that are being use I got from https://easings.net/
inline static float ApplyEase(float t, Ease _ease) {
	switch (_ease) {
	case CONSTANT:
		return t;
	case EASE_IN_CUBIC:
		return t * t * t;
	case EASE_OUT_CUBIC:
		return 1 - static_cast<float>(pow(1 - t, 3));
	case EASE_IN_ELASTIC:
		if (t == 0 || t == 1) {
			return t;
		} else {
			return -static_cast<float>(pow(2, 10 * t - 10)) * static_cast<float>(sin((t * 10 - 10.75) * (2 * SVEN_TWEEN_PI) / 3));
		}
	case EASE_OUT_ELASTIC:
		if (t == 0 || t == 1) {
			return t;
		} else {
			return static_cast<float>(pow(2, -10 * t)) * static_cast<float>(sin((t * 10 - 0.75) * (2 * SVEN_TWEEN_PI) / 3)) + 1;
		}
	case EASE_IN_HOLD:
		if (t < 0.3) {
			return 0;
		} else if (t < 0.7) {
			float norm_t = (t - 0.3f) / 0.4f;
			return norm_t * norm_t * norm_t;
		} else {
			return 1;
		}
	case EASE_IN_QUAD:
		return t * t;
	case EASE_OUT_QUAD:
		return 1 - static_cast<float>(pow(1 - t, 2));
	case EASE_IN_OUT_QUAD:
		if (t < 0.5) {
			return 2 * t * t;
		} else {
			return 1 - static_cast<float>(pow(-2 * t + 2, 2)) / 2;
		}
	case IN_OUT:
		if (t < 0.5) {
			return 2 * t;
		} else {
			return 1 - 2 * (t - 0.5f);
		}
	case EASE_OUT_CIRC:
		return static_cast<float>(sqrt(1 - static_cast<float>(pow(t - 1, 2))));
	case EASE_IN_QUINT:
		return t * t * t * t * t;
	case EASE_OUT_QUINT:
		return 1 - static_cast<float>(pow(1 - t, 5));
	default:
		return t;
	}
}