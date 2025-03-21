#pragma once
#include "precomp.h"

SceneManager::SceneManager() {
	activeScene = nullptr;
	nextScene = nullptr;
}

SceneManager::~SceneManager() {
}

void SceneManager::Init() {
}

void SceneManager::Update(float deltaTime) {
	if (nextScene != nullptr) {
		if (activeScene != nullptr) {
			activeScene->Exit();
			delete activeScene;
		}
		activeScene = nextScene;
		nextScene = nullptr;
		activeScene->Init();
	} else if (activeScene != nullptr) {
		activeScene->Update(deltaTime);
	}
}

void SceneManager::Render() {
	if (activeScene != nullptr) {
		activeScene->Render();
	}
}


void SceneManager::Destroy() {
	if (activeScene != nullptr) {
		activeScene->Exit();
		delete activeScene;
		activeScene = nullptr;
	}
	delete nextScene;
	nextScene = nullptr;
}

void SceneManager::LoadScene(GameScene* scene) {
	if (activeScene == nullptr) {
		activeScene = scene;
		activeScene->Init();
		return;
	} else {
		if (nextScene != nullptr) {
			delete nextScene;
			nextScene = nullptr;
		}
		nextScene = scene;
	}
}
