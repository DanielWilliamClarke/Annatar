#ifndef PLAYER_H
#define PLAYER_H
#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <map>

#include "entity/entity.h"

class IRenderer;
class IPlayerMovementComponent;
class IPlayerAttributeComponent;
struct Input;

struct Collision;
struct CollisionMediators;

template <typename T>
class IEntityObjectBuilder;

template<typename C, typename P>
class QuadTree;

enum class PlayerObjects { SHIP, EXHAUST, TURRET, GLOWIE };

class Player : public Entity<PlayerObjects>
{
public:
	enum movementStates : int { IDLE = 0, MOVING, MOVING_LEFT, MOVING_RIGHT, MOVING_UP, MOVING_DOWN };
	Player() = default;
	Player(
		std::shared_ptr<IEntityObjectBuilder<PlayerObjects>> playerBuilder,
		std::shared_ptr<IPlayerMovementComponent> movementComponent,
		std::shared_ptr<IPlayerAttributeComponent> attributeComponent,
		std::shared_ptr<ICollisionDetectionComponent> collisionDetectionComponent);
	virtual ~Player() = default;
	virtual void Update(std::shared_ptr<QuadTree<Collision, CollisionMediators>> quadTree, Input& in, float dt);
	virtual void Draw(std::shared_ptr<IRenderer> renderer, float interp) const override;
	virtual sf::Vector2f GetPosition() const override;

protected:
	virtual void Update(std::shared_ptr<QuadTree<Collision, CollisionMediators>> quadTree, float dt) override {};
	const unsigned int CalculateDirection(sf::Vector2f position, sf::Vector2f lastPosition) const;
	void InitBullets();

	std::shared_ptr<IPlayerMovementComponent> movementComponent;
	std::shared_ptr<IPlayerAttributeComponent> attributeComponent;
};

#endif //PLAYER_H