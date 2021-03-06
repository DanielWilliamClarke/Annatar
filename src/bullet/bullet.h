#ifndef BULLET_H
#define BULLET_H
#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <iostream>
#include <list>

class Entity;
struct EntityCollision;
class IRenderer;

template<typename U, typename C>
class QuadTree;

typedef QuadTree<Entity, EntityCollision> CollisionQuadTree;

enum class AFFINITY :int { LEFT = -1, RIGHT = 1 };

struct BulletConfig
{
	std::function<std::shared_ptr<sf::Shape>(void)> shapeBuilder;
	std::shared_ptr<Entity> owner;

	sf::Color color;
	float glowAttenuation;

	float rotation;
	float speed;
	AFFINITY affinity;

	bool penetrating;
	float damage;
	float lifeTime;

	BulletConfig(
		std::shared_ptr<Entity> owner,
		std::function<std::shared_ptr<sf::Shape>(void)> shapeBuilder,
		sf::Color color, 
		float glowAttenuation,
		float rotation, 
		float speed, 
		AFFINITY affinity,
		bool penetrating, 
		float damage, 
		float lifeTime = 20.0f)
		: owner(owner), 
		shapeBuilder(shapeBuilder),
		color(color),
		glowAttenuation(glowAttenuation),
		speed(speed),
		affinity(affinity), 
		rotation(rotation),
		penetrating(penetrating),
		damage(damage), 
		lifeTime(lifeTime)
	{}
};

struct BulletTrajectory
{
	sf::Vector2f position;
	sf::Vector2f velocity;
	float speed;

	BulletTrajectory(sf::Vector2f position, sf::Vector2f velocity, float speed)
		: position(position), velocity(velocity), speed(speed)
	{}
};

class Bullet
{
public:
	Bullet(BulletTrajectory& trajectory, BulletConfig& config);
	virtual ~Bullet() = default;

	virtual void Update(float dt, float worldSpeed) = 0;
	virtual void Draw(std::shared_ptr<IRenderer> renderer, float interp) = 0;
	virtual std::vector<std::shared_ptr<EntityCollision>> DetectCollisions(std::shared_ptr<CollisionQuadTree> quadTree) = 0;

	bool isSpent() const;
	sf::Vector2f GetPosition() const;
	sf::Vector2f GetVelocity() const;
	float GetDamage() const;
	std::shared_ptr<Entity> GetOwner() const;

protected:
	BulletConfig config;
	sf::Vector2f position;
	sf::Vector2f lastPosition;
	sf::Vector2f velocity;
	float speed;
	bool spent; // used in hit detection

	float accumulator;
	float minFadeout;
	float maxFadeout;
};

#endif // BULLET_H
