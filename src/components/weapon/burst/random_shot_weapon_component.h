#ifndef RANDOM_SHOT_WEAPON_COMPONENT_H
#define RANDOM_SHOT_WEAPON_COMPONENT_H

#include <SFML/Graphics.hpp>
#include <memory>

#include "components/weapon/i_weapon_component.h"
#include "util/i_random_number_source.h"

class IBulletSystem;
class IBulletFactory;

class RandomShotWeaponComponent
    : public IWeaponComponent,
      public WeaponBulletSystemAccess,
      public WeaponBulletFactoryAccess
{
public:
	RandomShotWeaponComponent(
        std::shared_ptr<IRandomNumberSource<int>> randSource,
        float numBullets
    );

	~RandomShotWeaponComponent() override = default;

    [[nodiscard]] WeaponState getWeaponState() const override;
	void Fire(sf::Vector2f position, BulletConfig& config) override;

private:
	std::shared_ptr<IRandomNumberSource<int>> randSource;
	float numBullets;
};

#endif //RANDOM_SHOT_WEAPON_COMPONENT_H