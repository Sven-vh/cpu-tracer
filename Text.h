#pragma once
#include "ICanvasElement.h"
#include <string>
#include <imgui/imgui.h>

//enum for text alignment
enum class TextAlignment {
	Left,
	Center,
	Right
};

class Text : public ICanvasElement {
public:
	Text(std::string text, float size, ImFont* font, TextAlignment alignment = TextAlignment::Center);
	~Text();
	// Inherited via ICanvasElement
	void Draw() override;
	void Update(float deltaTime) override;
	void Destroy() override;

	float GetWidth() const;
	float GetHeight() const;

	std::string TextString;
	float FontSize;
	float3 Color;
	ImFont* Font;
	TextAlignment Alignment = TextAlignment::Center;
};

