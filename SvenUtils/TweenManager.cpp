#include "TweenManager.h"

TweenManager::TweenManager() {

}

TweenManager::~TweenManager() {

}

void TweenManager::Update(float deltaTime) {
	//for each tween in tweens
	for (auto it = tweens.begin(); it != tweens.end(); ) {
		if (it == tweens.end()) {
			break;
		}
		(*it)->Update(deltaTime);
		if ((*it)->IsFinished()) {
			it = tweens.erase(it);
		} else {
			++it;
		}
	}
}

void TweenManager::AddTween(std::unique_ptr<ITween> tween) {
	tweens.push_back(std::move(tween));
}

void TweenManager::RemoveTween(std::unique_ptr<ITween>) {

}

void TweenManager::FixedUpdate(const float) {
	return;
}

void TweenManager::LateUpdate(const float) {
	return;
}

void TweenManager::Render() {
	return;
}

void TweenManager::Stop() {
	tweens.clear();
}

void TweenManager::Start() {
	return;
}
