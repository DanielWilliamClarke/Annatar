#define _USE_MATH_DEFINES
#include <cmath>

#include "enemy.h"

#include "../entity/i_entity_builder.h"
#include "../entity/entity_object.h"

#include "../components/movement/i_global_movement_component.h"

Enemy::Enemy(
	EntityManifest manifest,
	std::shared_ptr<IGlobalMovementComponent> globalMovementComponent,
	std::shared_ptr<IRandomNumberSource<int>> randSource)
	: Entity{ nullptr, globalMovementComponent }, randSource(randSource)
{
	this->objects = manifest;

	auto enemy = this->GetObject("enemy")->GetSprite();
	auto bounds = this->globalMovementComponent->GetBounds();
	enemy->setPosition({ bounds.width, (float)randSource->Generate(bounds.top, bounds.height)});
	this->globalMovementComponent->SetEntityAttributes(enemy->getPosition(), enemy->getGlobalBounds());
}

void Enemy::Update(float dt) const
{
	const auto position = this->globalMovementComponent->Integrate(dt);

	this->UpdateObjects({
		{ "enemy", EntityUpdate(position, IDLE) },
	}, dt);
}

void Enemy::Draw(sf::RenderTarget& target, float interp) const
{
	const auto interpPosition = this->globalMovementComponent->Interpolate(interp);
	this->DrawObjects(target, interpPosition);
}