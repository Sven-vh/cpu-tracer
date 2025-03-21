#include "KeyFrame.h"

KeyFrame::KeyFrame() {
	for (int i = 0; i < 256; i++) {
		keys[i] = false;
	}
	for (int i = 0; i < 5; i++) {
		mouseButtons[i] = false;
	}
	mousePosition = Vector2(0.0f, 0.0f);
	mouseWheelDelta = 0;
}

KeyFrame::~KeyFrame() {
}
