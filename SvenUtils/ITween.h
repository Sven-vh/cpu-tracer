#pragma once
class ITween {
public:
	virtual ~ITween() = default;
	virtual void Update(const float deltaTime) = 0;
	virtual bool IsFinished() const = 0;
};