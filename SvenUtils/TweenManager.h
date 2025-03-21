#pragma once
#include "Singleton.h"
#include "Manager.h"
#include "Tween.h"

#include <vector>
#include <memory>

using namespace std;

class TweenManager : public Singleton<TweenManager>, public IManager {
public:
	TweenManager();
	~TweenManager();

	void Start() override;
	void Update(const float deltaTime) override;

	void AddTween(std::unique_ptr<ITween> tween);
	void RemoveTween(std::unique_ptr<ITween> tween);

	// Inherited via Manager
	void FixedUpdate(const float fixedDeltaTime) override;
	void LateUpdate(const float deltaTime) override;
	void Render() override;
	void Stop() override;
private:
	std::vector<std::unique_ptr<ITween>> tweens;

};