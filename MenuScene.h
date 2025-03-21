#pragma once
#include "GameScene.h"
#include <functional>

#include "Canvas.h"
#include "Text.h"
#include "Image.h"

struct StartButton {
	float2 size;
	GLuint textureId;
	std::function<void()> onClick;

	StartButton(float2 size, std::string path, std::function<void()> onClick) {
		this->size = size;
		this->textureId = TextureFromFile(path.c_str());
		this->onClick = onClick;
	}

	~StartButton() {
	}

	ImVec2 GetImGuiSize() {
		return ImVec2(size.x, size.y);
	}
};

class MenuScene : public GameScene {
public:
	void Init() override;
	void Update(float deltaTime) override;
	void Render() override;
	void Exit() override;

	Renderer* renderer;
private:
	Canvas* canvas;
	std::vector<StartButton*> buttons;
	int scrWidth, scrHeight;
	int selectedLevel = 0;
	std::vector<Image*> levelImages;

	Image* cellularAutomataImage;
	StartButton* cellularAutomataButton;
	Image* earthImage;
	StartButton* earthButton;

	void RenderButtons();
	void UpdateBackground();
};

