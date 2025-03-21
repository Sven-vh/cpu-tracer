#include "InputManager.h"

InputManager::InputManager() {
}

InputManager::~InputManager() {
}

void InputManager::MouseUp(int button) {
	currentFrame.mouseButtons[button] = false;
}

void InputManager::MouseDown(int button) {
	currentFrame.mouseButtons[button] = true;
}

void InputManager::KeyUp(int key) {
	key = key;
	currentFrame.keys[key] = false;
}

void InputManager::KeyDown(int key) {
	key = key;
	currentFrame.keys[key] = true;
}

void InputManager::MouseMove(int x, int y) {
	currentFrame.mousePosition = Vector2(static_cast<float>(x), static_cast<float>(y));
}

void InputManager::MouseWheel(float delta) {
	currentFrame.mouseWheelDelta = delta;
}

void InputManager::Reset() {
	for (int i = 0; i < 256; i++) {
		currentFrame.keys[i] = false;
		previousFrame.keys[i] = false;
	}
	for (int i = 0; i < 3; i++) {
		currentFrame.mouseButtons[i] = false;
		previousFrame.mouseButtons[i] = false;
	}
	currentFrame.mousePosition = Vector2(0.0f, 0.0f);
	previousFrame.mousePosition = Vector2(0.0f, 0.0f);
}

bool InputManager::GetKeyDown(int key) {
	key = key;
	return currentFrame.keys[key] && !previousFrame.keys[key];
}

bool InputManager::GetKeyUp(int key) {
	key = key;
	return !currentFrame.keys[key] && previousFrame.keys[key];
}

bool InputManager::GetKey(int key) {
	key = key;
	return currentFrame.keys[key];
}

bool InputManager::GetMouseButtonDown(int button) {
	return currentFrame.mouseButtons[button] && !previousFrame.mouseButtons[button];
}

bool InputManager::GetMouseButtonUp(int button) {
	return !currentFrame.mouseButtons[button] && previousFrame.mouseButtons[button];
}

bool InputManager::GetMouseButton(int button) {
	return currentFrame.mouseButtons[button];
}

int InputManager::GetMouseWheelDelta() {
	return static_cast<int>(currentFrame.mouseWheelDelta);
}

Vector2 InputManager::GetMousePosition() {
	return currentFrame.mousePosition;
}

Vector2 InputManager::GetPreviousMousePosition() {
	return previousFrame.mousePosition;
}

Vector2 InputManager::GetMouseDelta() {
	return currentFrame.mousePosition - previousFrame.mousePosition;
}

void InputManager::Start() {
}

void InputManager::Update(const float) {
	previousFrame = currentFrame;
	currentFrame.mouseWheelDelta = 0;
}

void InputManager::FixedUpdate(const float) {
}

void InputManager::LateUpdate(const float) {

}

void InputManager::Render() {
}

void InputManager::Stop() {
}
