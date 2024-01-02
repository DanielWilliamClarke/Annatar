#include "single_shot_weapon_component.h"

#include "bullet/i_bullet_system.h"
#include "ui/i_player_hud.h"
#include "bullet/bullet.h"

SingleShotWeaponComponent::SingleShotWeaponComponent(
    std::shared_ptr<IBulletSystem> bulletSystem,
    std::shared_ptr<IBulletFactory> factory,
    std::shared_ptr<IPlayerHud> hud,
    WeaponSlot slot,
    float delay
)
	: IWeaponComponent(hud, slot), bulletSystem(bulletSystem), factory(factory), delay(delay), accumulator(0.0f)
{}

void SingleShotWeaponComponent::Fire(sf::Vector2f position, BulletConfig& config)
{
	this->accumulator += this->clockFire.restart().asSeconds();
	if (this->accumulator >= this->delay)
	{
		this->accumulator = 0;
		auto traj = BulletTrajectory(position, { 1 * (float)config.affinity, 0 }, config.speed);
		this->bulletSystem->FireBullet(factory, traj, config);
	}
}

WeaponState SingleShotWeaponComponent::getWeaponState() const
{
    return {
        "SingleShot",
        this->delay,
        this->accumulator,
        true
    };
}