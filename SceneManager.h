#pragma once

class SceneManager {
public:
	SceneManager();
	~SceneManager();
	void Init();
	void Update(float deltaTime);
	void Render();
	void Destroy();
	void LoadScene(GameScene* scene);

private:
	GameScene* activeScene;
	GameScene* nextScene;
};

