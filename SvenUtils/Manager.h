#pragma once
class IManager {
public:
	virtual void Start() = 0;
	
	virtual void Update(const float deltaTime) = 0;
	virtual void FixedUpdate(const float fixedDeltaTime) = 0;
	virtual void LateUpdate(const float deltaTime) = 0;

	virtual void Render() = 0;

	virtual void Stop() = 0;
};