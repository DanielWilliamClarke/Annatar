#ifndef SINGLE_SHOT_WEAPON_COMPONENT_H
#define SINGLE_SHOT_WEAPON_COMPONENT_H
#pragma once

#include <SFML/Graphics.hpp>

#include "i_weapon_component.h"

class IBulletSystem;

class SingleShotWeaponComponent: public IWeaponComponent
{
public:
	SingleShotWeaponComponent(std::shared_ptr<IBulletSystem> bulletSystem);
	virtual ~SingleShotWeaponComponent() = default;

	virtual void Fire(sf::Vector2f position) override;

private:
	std::shared_ptr<IBulletSystem> bulletSystem;
	sf::Clock clockFire;
	float accumulator;
	float delay;
};

#endif //SINGLE_SHOT_WEAPON_COMPONENT_H