#include "precomp.h"
#include "Text.h"

Text::Text(std::string text, float size, ImFont* font, TextAlignment alignment) {
	TextString = text;
	FontSize = size;
	Font = font;
	Alignment = alignment;
}

Text::~Text() {
}

void Text::Draw() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // Transparent button
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.1f)); // Slightly highlighted on hover
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.2f)); // More highlighted on active/click

    ImGui::Begin(TextString.c_str(), NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
    ImGui::PushFont(Font);
    // Set font size
    ImGui::SetWindowFontScale(FontSize / Font->FontSize);
    //set window pos
    ImGui::SetWindowPos(ImVec2(Position.x, Position.y));

    ImVec2 textSize = ImGui::CalcTextSize(TextString.c_str());
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 textStartPos;

    // Calculate text alignment and position
    switch (Alignment) {
    case TextAlignment::Left:
        textStartPos = ImVec2(0.0f + FontSize / 2.0f, (windowSize.y - textSize.y) / 2.0f);
        break;
    case TextAlignment::Center:
        textStartPos = ImVec2((windowSize.x - textSize.x) / 2.0f, (windowSize.y - textSize.y) / 2.0f);
        break;
    case TextAlignment::Right:
        textStartPos = ImVec2(windowSize.x - textSize.x, (windowSize.y - textSize.y) / 2.0f);
        break;
    }

    // Set cursor position based on alignment
    ImGui::SetCursorPos(textStartPos);
    ImGui::SetWindowSize(ImVec2(textSize.x + FontSize, textSize.y));
    ImGui::Text(TextString.c_str());

    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleColor(3); // Restore original style
}


void Text::Update(float) {
}

void Text::Destroy() {
}

float Text::GetWidth() const {
	float width = 0.0f;
	//for each character in the string get the size of the character from the font and add it to the width
	for (int i = 0; i < TextString.size(); i++) {
		width += Font->GetCharAdvance(TextString[i]);
	}
	return width;
}

float Text::GetHeight() const {
    return FontSize;
}
