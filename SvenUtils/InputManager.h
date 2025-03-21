#pragma once
#include "KeyFrame.h"
#include "Vector2.h"
#include "Singleton.h"
#include "Manager.h"
#include "KeyValues.h"

class InputManager : public Singleton<InputManager>, public IManager {
public:
	InputManager();
	~InputManager();

	// Inherited via IManager
	void Start() override;
	void Update(const float deltaTime) override;
	void FixedUpdate(const float fixedDeltaTime) override;
	void LateUpdate(const float deltaTime) override;
	void Render() override;
	void Stop() override;

	void MouseUp(int button);
	void MouseDown(int button);
	void KeyUp(int key);
	void KeyDown(int key);
	void MouseMove(int x, int y);
	void MouseWheel(float delta);

	void Reset();
	bool GetKeyDown(int key);
	bool GetKeyUp(int key);
	bool GetKey(int key);
	bool GetMouseButtonDown(int button);
	bool GetMouseButtonUp(int button);
	bool GetMouseButton(int button);
	int GetMouseWheelDelta();
	Vector2 GetMousePosition();
	Vector2 GetPreviousMousePosition();
	Vector2 GetMouseDelta();
private:
	KeyFrame currentFrame;
	KeyFrame previousFrame;
};
