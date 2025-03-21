#pragma once

class GameScene {
public:
	GameScene();
	virtual ~GameScene();
	virtual void Init() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render() = 0;
	virtual void Exit() = 0;

protected:
};

