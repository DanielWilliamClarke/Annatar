#ifndef BULLET_H
#define BULLET_H
#pragma once

#include <SFML/Graphics.hpp>
#include <functional>

class IGlowShaderRenderer;
class Entity;

struct BulletConfig
{
	std::function<std::shared_ptr<sf::Shape>(void)> shapeBuilder;
	std::shared_ptr<Entity> owner;
	sf::Color color;
	float rotation;

	float speed;
	bool penetrating;
	float damage;

	float lifeTime;

	BulletConfig(std::shared_ptr<Entity> owner, std::function<std::shared_ptr<sf::Shape>(void)> shapeBuilder, sf::Color color, float rotation, float speed, bool penetrating, float damage, float lifeTime = 0)
		: owner(owner), shapeBuilder(shapeBuilder), color(color), speed(speed), rotation(rotation), penetrating(penetrating), damage(damage), lifeTime(lifeTime)
	{}
};

class Bullet
{
public:
	Bullet(sf::Vector2f position, sf::Vector2f velocity, BulletConfig config);
	virtual ~Bullet() = default;

	void Update(float dt, float worldSpeed);
	void Draw(std::shared_ptr<IGlowShaderRenderer> renderer, float interp);

	void CollisionDetected();
	bool isSpent() const;

	std::shared_ptr<sf::Shape> GetRound() const;
	BulletConfig GetConfig() const;
	sf::Vector2f GetPosition() const;
	std::pair<float, bool> GetDamage() const;
	std::shared_ptr<Entity> GetOwner() const;

private:
	std::shared_ptr<sf::Shape> round; // Holds the bullet shape / position etc
	sf::Vector2f position;
	sf::Vector2f lastPosition;
	sf::Vector2f velocity;
	bool spent; // used in hit detection

	sf::Clock clock;
	float accumulator; 

	BulletConfig config;
};

#endif // BULLET_H
