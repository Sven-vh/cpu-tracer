#pragma once

//forward declaration
namespace Tmpl8 {
	class Renderer;
}

class PuzzleScene : public GameScene {
public:
	// Inherited via GameScene
	void Init() override;
	void Update(float deltaTime) override;
	void Render() override;
	void Exit() override;

	Renderer* renderer;
	int StartLevel = -1;

private:
	float lightSpeed = 0.1f;
	float fadeSpeed = 3.5f;
	std::vector <std::shared_ptr<PuzzleLevel>> levels;
	std::shared_ptr<PuzzleLevel> currentLevel;

	float allLampsFoundTime = 0;
	float allLampsFoundTimeMax = 1.0f;

	bool playAnimation = false;
	bool animationFinished = true;
	bool isRandomLevel = false;
	bool findRandomLevel = false;
	float animationRadius = 0.0f;
	float radiusSpeed = 5.0f;
	float totalTime;
	uint seed = 1234;

	float lightIntensity = 1.0f;

	void HandleMouse(const float deltaTime);
	void HandleLamps(const float deltaTime);

	void SetSimpleRenderer();
	void SetAdvancedRenderer();

	void LoadLevel(std::shared_ptr<PuzzleLevel> level);

	void DrawImGUI();

	void FinishLevelAnimation(const float deltaTime);

	bool GenerateRandomLevel();
};

