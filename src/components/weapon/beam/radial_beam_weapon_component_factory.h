#ifndef RADIAL_BEAM_WEAPON_COMPONENT_FACTORY_H
#define RADIAL_BEAM_WEAPON_COMPONENT_FACTORY_H


#include "components/weapon/i_weapon_component_factory.h"
#include "bullet/i_bullet_factory.h"

#include "radial_beam_weapon_component.h"

class RadialBeamWeaponComponentFactory: public IWeaponComponentFactory
{
public:
	RadialBeamWeaponComponentFactory(std::shared_ptr<IBulletFactory> factory, float duration, float arcAngle, float numBeams)
		: factory(factory),
		duration(duration),
		arcAngle(arcAngle),
		numBeams(numBeams),
		coolDown(0)
	{}

	~RadialBeamWeaponComponentFactory() override = default;

    [[nodiscard]] std::shared_ptr<IWeaponComponent> Construct(
        const std::shared_ptr<IBulletSystem>& bulletSystem,
        const std::shared_ptr<IPlayerHud>& hud,
        WeaponSlot slot,
        float delay
    ) const override {
        auto component = std::make_shared<RadialBeamWeaponComponent>(duration, delay, arcAngle, numBeams);

        component->setBulletSystem(bulletSystem);
        component->setBulletFactory(factory);
        component->setHud(hud);
        component->setSlot(slot);

        return component;
	};

private:
	std::shared_ptr<IBulletFactory> factory;
	float duration;
	float coolDown;
	float arcAngle;
	float numBeams;
};

#endif