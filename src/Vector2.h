#pragma once
#include "common.h"

template <typename T>
class Vector2
{
public:
	Vector2() : x(0), y(0) {}
	Vector2(T _x, T _y) : x(_x), y(_y) {}
	Vector2(const T val[2]) : x(val[0]), y(val[1]) {}

	T operator[](int i) const
	{
		if (i == 0) return x;
		return y;
	}
	T &operator[](int i)
	{
		if (i == 0) return x;
		return y;
	}
	Vector2<T> operator+(const Vector2<T> &v) const
	{
		return Vector2<T>(x + v.x, y + v.y);
	}
	Vector2<T> &operator+=(const Vector2<T> &v)
	{
		x += v.x; y += v.y;
		return *this;
	}
	Vector2<T> operator-(const Vector2<T> &v) const
	{
		return Vector2<T>(x - v.x, y - v.y);
	}
	Vector2<T> &operator-=(const Vector2<T> &v)
	{
		x -= v.x; y -= v.y;
		return *this;
	}
	Vector2<T> operator-() const
	{
		return Vector2<T>(-x, -y);
	}
	bool operator==(const Vector2<T> &v) const
	{
		return x == v.x && y == v.y;
	}
	bool operator!=(const Vector2<T> &v) const
	{
		return x != v.x || y != v.y;
	}
	template <typename U>
	Vector2<T> operator*(U s) const
	{
		return Vector2<T>(x * s, y * s);
	}
	template <typename U>
	friend Vector2<T> operator*(U s, const Vector2<T> &v)
	{
		return v * s;
	}
	template <typename U>
	Vector2<T> &operator*=(U s)
	{
		x *= s; y *= s;
		return *this;
	}
	template <typename U>
	Vector2<T> operator/(U f) const
	{
		float inv = (float)1 / f;
		return Vector2<T>(x * inv, y * inv);
	}
	template <typename U>
	Vector2<T> &operator/=(U f)
	{
		float inv = (float)1 / f;
		x *= inv; y *= inv;
		return *this;
	}

	float lengthSquared() const { return x * x + y * y; }
	float length() const { return std::sqrt(lengthSquared()); }
	void normalize() { *this /= length(); }

	friend std::ostream &operator<<(std::ostream &os, const Vector2<T> &v)
	{
		os << "[ " << v.x << ", " << v.y << " ]";
		return os;
	}
public:
	T x, y;
};

using Vector2f = Vector2<float>;
using Vector2i = Vector2<int32_t>;

template <typename T>
inline T cross(const Vector2<T> &v1, const Vector2<T> &v2)
{
	return v1.x * v2.y - v1.y * v2.x;
}

template <typename T>
inline T dot(const Vector2<T> &v1, const Vector2<T> &v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

template <typename T>
inline Vector2<T> normalize(const Vector2<T> &v)
{
	return v / v.length();
}