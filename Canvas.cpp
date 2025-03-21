#include "precomp.h"
#include "Canvas.h"

Canvas::Canvas() {
	elements = std::vector<ICanvasElement*>();
}

void Canvas::Draw() {
	for (ICanvasElement* element : elements) {
		element->Draw();
	}
}

void Canvas::Update(float deltaTime) {
	for (ICanvasElement* element : elements) {
		element->Update(deltaTime);
	}
}

void Canvas::Destroy() {
	for (ICanvasElement* element : elements) {
		element->Destroy();
	}
}

Canvas::~Canvas() {
	Destroy();
}

void Canvas::AddElement(ICanvasElement* element) {
	elements.push_back(element);
}

void Canvas::RemoveElement(ICanvasElement* element) {
	for (int i = 0; i < elements.size(); i++) {
		if (elements[i] == element) {
			elements.erase(elements.begin() + i);
			return;
		}
	}
}
