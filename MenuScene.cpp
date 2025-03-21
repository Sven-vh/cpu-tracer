#include "precomp.h"
#include "imgui_internal.h"


void MenuScene::Init() {
	renderer->IsInMenu = true;
	canvas = new Canvas();

	for (int i = 0; i < 10; i++) {
		StartButton* startButton = new StartButton(float2(200, 50), "assets/Buttons/Level_" + std::to_string(i + 1) + ".png", [this, i]() {
			renderer->IsInMenu = false;
			renderer->LoadLevel(i);
			});
		buttons.push_back(startButton);
	}

	for (int i = 0; i < 10; i++) {
		Image* levelImage = new Image("assets/LevelImages/Level_" + std::to_string(i + 1) + ".bmp");
		levelImage->ImageSize = float2(1920, 1016);
		levelImage->Position = float2(0, 0);
		levelImage->Invisible = true;
		canvas->AddElement(levelImage);
		levelImages.push_back(levelImage);
	}

	selectedLevel = 0;
	levelImages[selectedLevel]->Invisible = false;

	cellularAutomataImage = new Image("assets/LevelImages/Cellular_Automata.bmp");
	cellularAutomataImage->ImageSize = float2(2560, 1334);
	cellularAutomataImage->Position = float2(0, 0);
	cellularAutomataImage->Invisible = true;
	canvas->AddElement(cellularAutomataImage);

	earthImage = new Image("assets/LevelImages/Earth.bmp");
	earthImage->ImageSize = float2(1920, 1016);
	earthImage->Position = float2(0, 0);
	earthImage->Invisible = true;
	canvas->AddElement(earthImage);

	cellularAutomataButton = new StartButton(float2(200, 50), "assets/Buttons/Cellular.png", [this]() {
		renderer->IsInMenu = false;
		renderer->LoadCellularAutomata();
		});

	earthButton = new StartButton(float2(200, 50), "assets/Buttons/FreeRoam.png", [this]() {});
}

void MenuScene::Update(float deltaTime) {
	ImGuiIO& io = ImGui::GetIO();
	scrWidth = static_cast<int>(io.DisplaySize.x);
	scrHeight = static_cast<int>(io.DisplaySize.y);
	canvas->Update(deltaTime);
	UpdateBackground();
	if (InputManager::GetInstance().GetKeyDown(KeyValues::ESCAPE)) {
		QuitTemplate();
	}
}

void MenuScene::Render() {
	canvas->Draw();

	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(scrWidth), static_cast<float>(scrHeight)));
	//make the imgui window full screen
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	//get screen width and height from ImGui
	//transparent background and no interaction
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("Background", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);

	RenderButtons();
	ImGui::End();
}

void MenuScene::Exit() {
	canvas->Destroy();
}

void MenuScene::RenderButtons() {
	int numButtons = static_cast<int>(buttons.size());
	float margin = 15.0f; // Margin around buttons

	// Assuming all buttons have the same aspect ratio
	float originalButtonWidth = buttons[0]->size.x; // Original width of the button image
	float originalButtonHeight = buttons[0]->size.y; // Original height of the button image
	float aspectRatio = originalButtonHeight / originalButtonWidth; // Aspect ratio of the button image

	float buttonWidth = originalButtonWidth; // Set the width for the buttons (can be adjusted as needed)
	float buttonHeight = buttonWidth * aspectRatio; // New button height maintaining aspect ratio

	// Scrollable section size and position
	ImVec2 sectionSize = ImVec2(buttonWidth + margin * 3, ImGui::GetIO().DisplaySize.y); // Full height, width based on button width
	ImVec2 sectionPos = ImVec2(0, 0); // Starting at top-left corner

	// Begin scrollable section
	ImGui::SetNextWindowPos(sectionPos);
	ImGui::SetNextWindowSize(sectionSize);
	ImGui::Begin("ScrollableSection", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
	// Create a child window to enable scrolling
	ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
	ImGui::BeginChild("Scrolling", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
	ImVec2 buttonSize = ImVec2(buttonWidth, buttonHeight);

	// Render buttons
	for (int i = 0; i < numButtons; ++i) {
		StartButton* button = buttons[i];

		// Positioning not required, ImGui will layout buttons vertically

		//no background button
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		// Render button with image
		if (ImGui::ImageButton((void*)(intptr_t)button->textureId, buttonSize)) {
			button->onClick();
		}

		if (ImGui::IsItemHovered()) {
			selectedLevel = i;
			levelImages[selectedLevel]->Invisible = false;
			earthImage->Invisible = true;
			cellularAutomataImage->Invisible = true;
		}

		// Add some space between buttons
		ImGui::Dummy(ImVec2(0.0f, margin));
		// Restore original style	
		ImGui::PopStyleColor();
	}


	// End child and section
	ImGui::EndChild();
	ImGui::End();
	buttonSize.x *= 2.0f;
	buttonSize.y *= 2.0f;

	//new Imgui window with 2 buttons on the bottom right
	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(scrWidth) - buttonSize.x * 2.0f, static_cast<float>(scrHeight) - buttonSize.y - margin * 3));
	ImGui::SetNextWindowSize(ImVec2(buttonSize.x * 2.0f + margin * 3, buttonSize.y * 2.0f));
	ImGui::Begin("Exit Window", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
	ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	if (ImGui::ImageButton((void*)(intptr_t)cellularAutomataButton->textureId, buttonSize)) {
		renderer->IsInMenu = false;
		renderer->LoadCellularAutomata();
	}
	if (ImGui::IsItemHovered()) {
		cellularAutomataImage->Invisible = false;
		earthImage->Invisible = true;
		levelImages[selectedLevel]->Invisible = true;
	}
	ImGui::SameLine();
	if (ImGui::ImageButton((void*)(intptr_t)earthButton->textureId, buttonSize)) {
		renderer->IsInMenu = false;
		renderer->LoadFreeCamera();
	}
	if (ImGui::IsItemHovered()) {
		earthImage->Invisible = false;
		cellularAutomataImage->Invisible = true;
		levelImages[selectedLevel]->Invisible = true;
	}
	ImGui::PopStyleColor();
	ImGui::End();
}

void MenuScene::UpdateBackground() {
	for (int i = 0; i < levelImages.size(); i++) {
		Image* background = levelImages[i];
		if (i != selectedLevel) background->Invisible = true;
		const float imageAspectRatio = background->ImageSize.x / background->ImageSize.y;
		const float screenAspectRatio = (float)scrWidth / (float)scrHeight;

		float scaleFactor;
		if (screenAspectRatio > imageAspectRatio) {
			// Screen is wider than the image: scale image based on width
			scaleFactor = scrWidth / background->ImageSize.x;
		} else {
			// Screen is taller than the image: scale image based on height
			scaleFactor = scrHeight / background->ImageSize.y;
		}

		background->ImageSize = float2(background->ImageSize.x * scaleFactor, background->ImageSize.y * scaleFactor);
		background->Size = float2(static_cast<float>(scrWidth), static_cast<float>(scrHeight));
	}

	const float imageAspectRatio = cellularAutomataImage->ImageSize.x / cellularAutomataImage->ImageSize.y;
	const float screenAspectRatio = (float)scrWidth / (float)scrHeight;

	float scaleFactor;
	if (screenAspectRatio > imageAspectRatio) {
		// Screen is wider than the image: scale image based on width
		scaleFactor = scrWidth / cellularAutomataImage->ImageSize.x;
	} else {
		// Screen is taller than the image: scale image based on height
		scaleFactor = scrHeight / cellularAutomataImage->ImageSize.y;
	}

	cellularAutomataImage->ImageSize = float2(cellularAutomataImage->ImageSize.x * scaleFactor, cellularAutomataImage->ImageSize.y * scaleFactor);
	cellularAutomataImage->Size = float2(static_cast<float>(scrWidth), static_cast<float>(scrHeight));

	const float imageAspectRatio2 = earthImage->ImageSize.x / earthImage->ImageSize.y;
	const float screenAspectRatio2 = (float)scrWidth / (float)scrHeight;

	float scaleFactor2;
	if (screenAspectRatio2 > imageAspectRatio2) {
		// Screen is wider than the image: scale image based on width
		scaleFactor2 = scrWidth / earthImage->ImageSize.x;
	} else {
		// Screen is taller than the image: scale image based on height
		scaleFactor2 = scrHeight / earthImage->ImageSize.y;
	}

	earthImage->ImageSize = float2(earthImage->ImageSize.x * scaleFactor2, earthImage->ImageSize.y * scaleFactor2);
	earthImage->Size = float2(static_cast<float>(scrWidth), static_cast<float>(scrHeight));
}
