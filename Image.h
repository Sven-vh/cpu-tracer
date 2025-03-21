#pragma once
#include "ICanvasElement.h"
#include <string>

class Image : public ICanvasElement {
public:
	Image(std::string path);
	~Image();
	// Inherited via ICanvasElement
	void Draw() override;
	void Update(float deltaTime) override;
	void Destroy() override;

	float2 ImageSize;
private:
	std::string path;
	GLuint texture;
};

