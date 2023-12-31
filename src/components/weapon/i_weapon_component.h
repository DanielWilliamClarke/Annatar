#ifndef I_WEAPON_COMPONENT_H
#define I_WEAPON_COMPONENT_H

#include <SFML/Graphics.hpp>

struct BulletConfig;

class IWeaponComponent
{
public:
	IWeaponComponent() = default;
	virtual ~IWeaponComponent() = default;

	virtual void Fire(sf::Vector2f position, BulletConfig& config) = 0;
	virtual void Cease() {};
};

#endif