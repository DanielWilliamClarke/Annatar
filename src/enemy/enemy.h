#ifndef ENEMY_H
#define ENEMY_H
#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

#include "../entity/entity.h"

#include "../util/i_random_number_source.h"


class Enemy : public Entity
{
public:
	enum movementStates { IDLE = 0 };
	Enemy() = default;
	Enemy(
		EntityManifest manifest,
		std::shared_ptr<IGlobalMovementComponent> movementComponent,
		sf::Vector2f initialPosition);
	virtual ~Enemy() = default;
	virtual void Update(float dt) const override;
	virtual void Draw(sf::RenderTarget& target, float interp) const override;
};

#endif //ENEMY_H