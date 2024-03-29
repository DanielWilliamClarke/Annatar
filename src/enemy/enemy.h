#ifndef ENEMY_H
#define ENEMY_H

#include <SFML/Graphics.hpp>
#include <memory>

#include "entity/entity.h"

class IRenderer;

enum class EnemyObjects { ENEMY };

class Enemy : public Entity<EnemyObjects>
{
public:
	enum movementStates { IDLE = 0 };
	Enemy() = default;
	Enemy(
		std::unordered_map<EnemyObjects, std::shared_ptr<EntityObject>> manifest,
		std::shared_ptr<IGlobalMovementComponent> movementComponent,
		std::shared_ptr<IAttributeComponent> attributeComponent,
		std::shared_ptr<ICollisionDetectionComponent> collisionDetectionComponent,
		sf::Vector2f initialPosition);
	virtual ~Enemy() = default;
	virtual void Update(const CollisionQuadTree& quadTree, float dt) override;
	virtual void Draw(const std::shared_ptr<IRenderer>& renderer, float interp) const override;
private:
	void InitBullets();

	std::shared_ptr<CollisionMediators> mediators;
};

#endif //ENEMY_H