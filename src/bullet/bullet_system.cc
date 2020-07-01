#include "bullet_system.h"
#include "bullet.h"

#include "../entity/entity.h"

BulletSystem::BulletSystem(sf::FloatRect bounds, std::vector<std::shared_ptr<Entity>> collisionTargets)
	: bounds(bounds), collisionTargets(collisionTargets)
{}

void BulletSystem::FireBullet(sf::Vector2f position, sf::Vector2f velocity, sf::Color colour, float radius)
{
	this->bullets.push_back(Bullet(position, velocity, colour, radius));
}

void BulletSystem::Update(float dt, float worldSpeed)
{
	// Update and perform collision detection
	for (auto& b : this->bullets)
	{
		b.Update(dt, worldSpeed);

		for (auto& c : collisionTargets)
		{
			

			if (c->DetectCollision(b.GetRound().getGlobalBounds()))
			{
				// update entity

				// spend round 
				b.CollisionDetected();
			}
		}
	}

	// Remove bullets 
	this->bullets.erase(
		std::remove_if(this->bullets.begin(), this->bullets.end(), [=](Bullet b) -> bool {
			auto position = b.GetRound().getPosition();
			return position.x <= bounds.left ||	position.x >= bounds.width ||
				position.y <= bounds.top ||	position.y >= bounds.height ||
				b.isSpent();
			}),
		this->bullets.end());
}

void BulletSystem::Draw(sf::RenderTarget& target, float interp)
{
	for (auto& b : this->bullets)
	{
		b.Draw(target, interp);
	}
}
