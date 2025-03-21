#pragma once

class ICanvasElement {
public:
	virtual void Draw() = 0;
	virtual void Update(float deltaTime) = 0;

	virtual void Destroy() = 0;

	virtual ~ICanvasElement() {}

	float2 Position;
	float2 Size;

	bool Invisible = false;

protected:
	ICanvasElement();
};