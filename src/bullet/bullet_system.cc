#include "bullet_system.h"

#include <algorithm>

#include "types/projectile.h"

#include "i_bullet_factory.h"
#include "entity/entity.h"
#include "components/weapon/i_weapon_component.h"
#include "components/hitbox/i_hitbox_component.h"

#include "quad_tree/quad_tree.h"
#include "util/container_utils.h"
#include "util/i_threaded_workload.h"

struct UnresolvedCollisions
{
	std::shared_ptr<Bullet> bullet;
	std::vector<std::shared_ptr<EntityCollision>> collisions;

	UnresolvedCollisions(std::shared_ptr<Bullet> b, std::vector<std::shared_ptr<EntityCollision>> c)
		: bullet(b), collisions(c)
	{}
};

BulletSystem::BulletSystem(sf::FloatRect bounds)
	: bounds(bounds)
{}

std::shared_ptr<Bullet> BulletSystem::FireBullet(std::shared_ptr<IBulletFactory> factory, BulletTrajectory& trajectory, BulletConfig& config)
{
	auto bullet = factory->Construct(trajectory, config);
	this->AddBullet(bullet);
	return bullet;
}

void BulletSystem::Update(std::shared_ptr<CollisionQuadTree> quadTree, float dt, float worldSpeed)
{
	this->EraseBullets();

	// Determine collisions to resolve
	std::vector<UnresolvedCollisions> unresolved;
	for (auto& b : this->bullets)
	{
		b->Update(dt, worldSpeed);

		//quadTree->Insert(
		//	Point< std::shared_ptr<Entity>>(b->GetPosition(), b->GetOwner()));

		auto collisions = b->DetectCollisions(quadTree);
		if (collisions.size() && b->GetDamage() > 0.0f)
		{
			unresolved.push_back(UnresolvedCollisions(b, collisions));
		}
	}

	// Resolve each collision
	std::for_each(unresolved.begin(), unresolved.end(),
		[&](UnresolvedCollisions& u) {
			auto damage = u.bullet->GetDamage();
			for (auto& collision : u.collisions)
			{
				collision->target->TakeDamage(damage, collision->point);
				if (u.bullet->GetOwner() && collision->target->HasDied())
				{
					u.bullet->GetOwner()->RegisterKill(damage);
				}
			}
		});
}

void BulletSystem::Draw(std::shared_ptr<IRenderer> renderer, float interp)
{
	for (auto& b : this->bullets)
	{
		b->Draw(renderer, interp);
	}
}

void BulletSystem::AddBullet(std::shared_ptr<Bullet> bullet)
{
	this->bullets.push_back(bullet);
}

void BulletSystem::EraseBullets()
{
	this->bullets.erase(
		std::remove_if(
			this->bullets.begin(), this->bullets.end(),
			[this](std::shared_ptr<Bullet>& b) -> bool {
				return b->isSpent();
			}),
		this->bullets.end());
}