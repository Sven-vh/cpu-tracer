#include "precomp.h"

Image::Image(std::string path) {
	this->path = path;
	texture = TextureFromFile(path.c_str());
}

Image::~Image() {
}

void Image::Draw() {
	if(Invisible) return;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | 
		ImGuiWindowFlags_NoMove | 
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_NoCollapse | 
		ImGuiWindowFlags_NoInputs | 
		ImGuiWindowFlags_NoSavedSettings | 
		ImGuiWindowFlags_NoFocusOnAppearing | 
		ImGuiWindowFlags_NoBringToFrontOnFocus | 
		ImGuiWindowFlags_NoNav | 
		ImGuiWindowFlags_NoNavFocus | 
		ImGuiWindowFlags_NoNavInputs | 
		ImGuiWindowFlags_NoNavFocus | 
		ImGuiWindowFlags_NoBackground | 
		ImGuiWindowFlags_NoTitleBar | 
		ImGuiWindowFlags_NoScrollbar | 
		ImGuiWindowFlags_NoScrollWithMouse | 
		ImGuiWindowFlags_NoMouseInputs | 
		ImGuiWindowFlags_NoFocusOnAppearing | 
		ImGuiWindowFlags_NoBringToFrontOnFocus | 
		ImGuiWindowFlags_NoNav | 
		ImGuiWindowFlags_NoNavFocus | 
		ImGuiWindowFlags_NoNavInputs | 
		ImGuiWindowFlags_NoNavFocus | 
		ImGuiWindowFlags_NoBackground | 
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoScrollbar | 
		ImGuiWindowFlags_NoScrollWithMouse | 
		ImGuiWindowFlags_NoMouseInputs;
	ImGui::Begin(path.c_str(), nullptr, window_flags);
	ImGui::SetWindowPos(ImVec2(Position.x, Position.y));
	ImGui::SetWindowSize(ImVec2(Size.x, Size.y));

	ImVec2 topLeftPos = ImVec2(
		(Size.x - ImageSize.x) * 0.5f, // Center horizontally
		(Size.y - ImageSize.y) * 0.5f  // Center vertically
	);

	ImGui::SetCursorPos(topLeftPos);

	ImGui::Image((void*)(intptr_t)texture, ImVec2(ImageSize.x, ImageSize.y));
	ImGui::End();
}

void Image::Update(float) {
}

void Image::Destroy() {
}
