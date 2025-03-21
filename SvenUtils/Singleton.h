#pragma once
#ifndef SINGLETON_H
#define SINGLETON_H

template <typename T>
class Singleton {
protected:
	Singleton() = default;
	virtual ~Singleton() = default;

public:
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

	static T& GetInstance() {
		static T instance;
		return instance;
	}
};
#endif // SINGLETON_H
