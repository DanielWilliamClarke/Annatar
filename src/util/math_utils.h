#ifndef MATH_HELPERS_H
#define MATH_HELPERS_H

#include <SFML/Graphics.hpp>
#include <numbers>
#include <cmath>

class AngleConversion
{
public:
	static float ToDegrees(float radians)
	{
		return radians * (180.0f / (float)std::numbers::pi);
	}

	static float ToRadians(float degrees)
	{
		return degrees * ((float)std::numbers::pi / 180.0f);
	}
};

class Dimensions
{
public:
	static float DistanceBetween(sf::Vector2f start, sf::Vector2f end)
	{
		auto dx = start.x - end.x;
		auto dy = start.y - end.y;
		return sqrtf(dx * dx + dy * dy);
	}

	static float ManhattanDistance(sf::Vector2f start, sf::Vector2f end)
	{
		auto dx = abs(start.x - end.x);
		auto dy = abs(start.y - end.y);
		return dx + dy;
	}

	static float Magnitude(sf::Vector2f point)
	{
		return sqrtf((point.x * point.x) + (point.y * point.y));
	}

	static sf::Vector2f Normalise(sf::Vector2f point)
	{
		return point / Magnitude(point);
	}
};

#endif // MATH_HELPERS_H



