#include "precomp.h"

void CellularAutomata::Init() {
	renderer->CanSelectWorld = true;
}

void CellularAutomata::Update(float) {
	if (InputManager::GetInstance().GetKeyDown(KeyValues::ESCAPE)) {
		MenuScene* menu = new MenuScene();
		menu->renderer = renderer;
		renderer->sceneManager.LoadScene(menu);
	}
}

void CellularAutomata::Render() {
}

void CellularAutomata::Exit() {
}
