#pragma once
#include "Vector2.h"

class KeyFrame {
public:
	KeyFrame();
	~KeyFrame();

	bool keys[512];
	bool mouseButtons[5];
	Vector2 mousePosition;
	float mouseWheelDelta;
};

