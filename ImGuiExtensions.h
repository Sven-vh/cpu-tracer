#pragma once
static inline bool DirectionProperties(float3& direction, int index) {
	// Calculate initial up and down from the current direction vector
	float up = atan2f(direction.z, direction.x) * 180.0f / TWEEN_PI; // up from X-axis in XZ-plane
	if (up < 0) up += 360.0f; // Ensure up is in the range [0, 360]
	float down = asinf(direction.y) * 180.0f / TWEEN_PI; // down from XY-plane to Y-axis
	bool updateDirection = false;

	std::string leftRightLabel = "Left/Right##" + std::to_string(index);
	std::string upDownLabel = "Up/Down##" + std::to_string(index);

	if (ImGui::SliderFloat(leftRightLabel.c_str(), &up, 0.0f, 360.0f)) updateDirection = true;
	if (ImGui::SliderFloat(upDownLabel.c_str(), &down, -89.999f, 89.999f)) updateDirection = true;

	// Update direction vector when the "Update Direction" button is clicked
	if (updateDirection) {
		// Convert up and down back to Cartesian coordinates
		float upRad = up * TWEEN_PI / 180.0f; // Convert degrees to radians
		float downRad = down * TWEEN_PI / 180.0f; // Convert degrees to radians

		direction.x = cosf(downRad) * cosf(upRad);
		direction.y = sinf(downRad);
		direction.z = cosf(downRad) * sinf(upRad);
		return true;
	}
	return false;
}

static inline bool SurfaceDropDown(Surface* surface, int) {
	//Read all the .png files in the assets folder and store them in a vector
	std::vector<std::string> hdrFiles;
	for (const auto& entry : std::filesystem::directory_iterator("assets")) {
		if (entry.path().extension() == ".png") {
			hdrFiles.push_back(entry.path().filename().string());
		}
	}

	// Dropdown for selecting .png file
	static int selectedItem = -1; // Index of the selected item in the combo box
	if (!hdrFiles.empty()) {
		std::vector<const char*> cStrHdrFiles; // ImGui needs const char* array
		for (const auto& file : hdrFiles) {
			cStrHdrFiles.push_back(file.c_str());
		}

		ImGui::Combo("HDR Files", &selectedItem, &cStrHdrFiles[0], static_cast<int>((cStrHdrFiles.size())));
	}

	if (ImGui::Button("Load HDR File") && selectedItem >= 0) {
		// Load the selected .png file
		std::string path = "assets/" + hdrFiles[selectedItem];
		if (surface->ownBuffer) FREE64(surface->pixels);
		surface->LoadFromFile(path.c_str());
		return true;
	}
	return false;
}

static inline bool SurfaceDropDown(FLoatSurface* surface, int index) {
	//Read all the .png files in the assets folder and store them in a vector
	std::vector<std::string> hdrFiles;
	for (const auto& entry : std::filesystem::directory_iterator("assets")) {
		if (entry.path().extension() == ".png" || entry.path().extension() == ".hdr") {
			hdrFiles.push_back(entry.path().filename().string());
		}
	}

	// Dropdown for selecting .png file
	static int selectedItem = -1; // Index of the selected item in the combo box
	std::string label = "Files##" + std::to_string(index);
	if (!hdrFiles.empty()) {
		std::vector<const char*> cStrHdrFiles; // ImGui needs const char* array
		for (const auto& file : hdrFiles) {
			cStrHdrFiles.push_back(file.c_str());
		}

		ImGui::Combo(label.c_str(), &selectedItem, &cStrHdrFiles[0], static_cast<int>(cStrHdrFiles.size()));
	}

	std::string loadLabel = "Load File##" + std::to_string(index);
	if (ImGui::Button(loadLabel.c_str()) && selectedItem >= 0) {
		// Load the selected .png file
		std::string path = "assets/" + hdrFiles[selectedItem];
		if (surface == nullptr) {
			surface = new FLoatSurface();
		} else {
			if (surface->ownBuffer) FREE64(surface->pixels);
		}
		surface->LoadFromFile(path.c_str());
		//print the size of the pixels buffer
		std::cout << "Size of the pixels buffer: " << surface->width << "x" << surface->height << std::endl;
		return true;
	}
	return false;
}

static inline bool uintColorPicker(const char* label, uint& color) {
	float col[3];
	col[0] = (float)((color >> 16) & 0xFF) / 255.0f;
	col[1] = (float)((color >> 8) & 0xFF) / 255.0f;
	col[2] = (float)((color >> 0) & 0xFF) / 255.0f;
	if (ImGui::ColorEdit3(label, col)) {
		color = ((uint32_t)(col[0] * 255.0f) << 16) | ((uint32_t)(col[1] * 255.0f) << 8) | (uint32_t)(col[2] * 255.0f);
		return true;
	}
	return false;
}

static inline bool intColorPicker(const char* label, int& color) {
	uint c = (uint)color;
	bool changed = uintColorPicker(label, c);
	color = (int)c;
	return changed;
}

static inline bool MatrixProperties(const char*, const mat4& matrix) {
	bool updateMatrix = false;
	mat4 m = matrix;
	ImGui::NewLine();
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			ImGui::SameLine();
			std::string id = std::to_string(y) + std::to_string(x);
			int index = y * 4 + x;
			//set width of the input field to 50
			ImGui::SetNextItemWidth(50);
			if (ImGui::InputFloat(id.c_str(), &m[index])) updateMatrix = true;
		}
		ImGui::NewLine();
	}
	return updateMatrix;
}