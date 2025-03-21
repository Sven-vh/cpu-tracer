#pragma once
#include <vector>
#include "ICanvasElement.h"

class Canvas {
public:
	Canvas();
	void Draw();
	void Update(float deltaTime);

	void Destroy();

	~Canvas();

	void AddElement(ICanvasElement* element);
	void RemoveElement(ICanvasElement* element);

private:
	std::vector<ICanvasElement*> elements;
};