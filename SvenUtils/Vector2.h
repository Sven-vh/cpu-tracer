#pragma once
#include <math.h> 
#include <iostream>

//Made my own Vector2 class
struct Vector2 {

	float x, y;
	static const Vector2 Zero;
	static const Vector2 One;
	static const Vector2 Up;
	static const Vector2 Down;
	static const Vector2 Left;
	static const Vector2 Right;

	Vector2() {
		x = 0; y = 0;
	}
	Vector2(float x, float y) {
		this->x = x;
		this->y = y;
	}

	//deconstructor
	~Vector2() {
	}

	//bool isNull() const {
	//	return std::isnan(x) || std::isnan(y);
	//}

	Vector2 operator+ (Vector2 adder) {
		return Vector2(x + adder.x, y + adder.y);
	}
	void operator+= (Vector2 adder) {
		x += adder.x; y += adder.y;
	}

	Vector2 operator+ (float adder) {
		return Vector2(x + adder, y + adder);
	}

	void operator+= (float adder) {
		x += adder; y += adder;
	}

	Vector2 Normalize() {
		float length = sqrt(x * x + y * y);

		if (length == 0.0f) return Vector2(0, 0);
		return Vector2(x / length, y / length);
	}

	Vector2 operator- (Vector2 subtracter) {
		return Vector2(x - subtracter.x, y - subtracter.y);
	}
	void operator-= (Vector2 subtracter) {
		x -= subtracter.x; y -= subtracter.y;
	}

	Vector2 operator- (float subtracter) {
		return Vector2(x - subtracter, y - subtracter);
	}
	void operator-= (float subtracter) {
		x -= subtracter; y -= subtracter;
	}

	Vector2 operator* (Vector2 scaler) {
		return Vector2(x * scaler.x, y * scaler.y);
	}
	void operator*= (Vector2 scaler) {
		x *= scaler.x; y *= scaler.y;
	}

	Vector2 operator* (float scaler) {
		return Vector2(x * scaler, y * scaler);
	}
	void operator*= (float scaler) {
		x *= scaler; y *= scaler;
	}

	Vector2 operator- () {
		return Vector2(-x, -y);
	}

	Vector2 operator/ (Vector2 scaler) {
		return Vector2(x / scaler.x, y / scaler.y);
	}
	void operator/= (Vector2 scaler) {
		x /= scaler.x; y /= scaler.y;
	}

	Vector2 operator/ (float scaler) {
		return Vector2(x / scaler, y / scaler);
	}
	void operator/= (float scaler) {
		x /= scaler; y /= scaler;
	}

	bool operator== (Vector2 other) {
		return x == other.x && y == other.y;
	}

	bool operator!= (Vector2 other) {
		return x != other.x || y != other.y;
	}

	bool operator> (Vector2 other) {
		return x > other.x && y > other.y;
	}

	bool operator< (Vector2 other) {
		return x < other.x && y < other.y;
	}

	bool operator>= (Vector2 other) {
		return x >= other.x && y >= other.y;
	}

	bool operator<= (Vector2 other) {
		return x <= other.x && y <= other.y;
	}

	void operator++ () {
		x++; y++;
	}

	void operator-- () {
		x--; y--;
	}

	Vector2 operator%(const Vector2& other) const {
		return Vector2(fmod(x, other.x), fmod(y, other.y));
	}

	Vector2 absolute() {
		return Vector2(fabs(x), fabs(y));
	}

	//static float Distance(Vector2 a, Vector2 b) {
	//	float distanceX = b.x - a.x;
	//	float distanceY = b.y - a.y;
	//	return sqrt(distanceX * distanceX + distanceY * distanceY);
	//}

	//static Vector2 pow(Vector2 v, const float exponent) {
	//	return Vector2(std::pow(v.x, exponent), std::pow(v.y, exponent));
	//}

	//Vector2 pow(const float exponent) {
	//	return Vector2(std::pow(x, exponent), std::pow(y, exponent));
	//}

	static Vector2 Lerp(Vector2& a, Vector2& b, float t) {
		return a + (b - a) * t;
	}

	float Dot(Vector2 b) {
		return x * b.x + y + b.y;
	}

	float Length() {
		return sqrt(x * x + y * y);
	}

	float Magnitude() {
		return sqrt(x * x + y * y);
	}

	float Dot(Vector2 a, Vector2 b) {
		return a.x * b.x + a.y * b.y;
	}

	void Log() {
		printf("(%f, %f)\n", x, y);
	}

	void Clamp(Vector2 min, Vector2 max) {
		if (x < min.x) x = min.x;
		if (y < min.y) y = min.y;
		if (x > max.x) x = max.x;
		if (y > max.y) y = max.y;
	}

	void Lerp(Vector2 b, float t) {
		x = x + (b.x - x) * t;
		y = y + (b.y - y) * t;
	}
};