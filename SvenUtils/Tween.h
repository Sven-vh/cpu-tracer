#pragma once
#include <functional>
#include "ITween.h"
#include "TweenUtils.h"

template<typename T>
class Tween : public ITween {
public:
	Tween();
	Tween(T& adress, T endValue, const float duration);
	~Tween();

	Tween& OnUpdate(const std::function<void(T, float)> _onUpdate) {
		this->onUpdate = _onUpdate;
		return *this;
	}

	Tween& OnFinish(const std::function<void(T)> _onComplete) {
		this->onComplete = _onComplete;
		return *this;
	}

	Tween& SetEase(Ease newEase) {
		this->ease = newEase;
		return *this;
	}

protected:

	void Update(const float deltaTime) override;

	bool IsFinished() const override;

private:
	T& adress;
	T startValue;
	T endValue;
	Ease ease;
	float duration;
	float elapsed;

	std::function<void(T, float)> onUpdate;
	std::function<void(T)> onComplete;
};

template<typename T>
inline Tween<T>::Tween() {
}

template<typename T>
inline Tween<T>::Tween(T& adress, T endValue, const float duration)
	: adress(adress),
	startValue(adress),
	endValue(endValue),
	duration(duration),
	elapsed(0),
	ease(Ease::CONSTANT),
	onUpdate([](T, float) {}),
	onComplete([](T) {}) {

}

template<typename T>
inline Tween<T>::~Tween() {
}

template<typename T>
inline void Tween<T>::Update(const float deltaTime) {
	elapsed += deltaTime;
	float t = elapsed / duration;
	t = ApplyEase(t, ease);
	adress = lerp(startValue, endValue, t);
	onUpdate(adress, t);
	if (IsFinished()) {
		adress = endValue;
		onComplete(adress);
	}
}

template<typename T>
inline bool Tween<T>::IsFinished() const {
	return elapsed >= duration;
}
