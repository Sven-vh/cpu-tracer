#pragma once
#include "GameScene.h"
class CellularAutomata : public GameScene {
public:
	// Inherited via GameScene
	void Init() override;
	void Update(float deltaTime) override;
	void Render() override;
	void Exit() override;

	Renderer* renderer;
};

