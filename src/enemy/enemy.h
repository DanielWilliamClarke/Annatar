#ifndef ENEMY_H
#define ENEMY_H
#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

#include "entity/entity.h"

class IRenderer;

template<typename U, typename C>
class QuadTree;

class Enemy : public Entity
{
public:
	enum movementStates { IDLE = 0 };
	Enemy() = default;
	Enemy(
		EntityManifest manifest,
		std::shared_ptr<IGlobalMovementComponent> movementComponent,
		std::shared_ptr<IAttributeComponent> attributeComponent,
		sf::Vector2f initialPosition);
	virtual ~Enemy() = default;
	virtual void Update(std::shared_ptr<CollisionQuadTree> quadTree, float dt) override;
	virtual void Draw(std::shared_ptr<IRenderer> renderer, float interp) const override;
	virtual sf::Vector2f GetPosition() const override;
	virtual bool IsInside(sf::FloatRect& area) const override;

private:
	void InitBullets();
};

#endif //ENEMY_H