#include "random_shot_weapon_component.h"

#include "util/math_utils.h"

#include "bullet/i_bullet_system.h"
#include "bullet/i_bullet_factory.h"
#include "bullet/bullet.h"
#include "ui/i_player_hud.h"

RandomShotWeaponComponent::RandomShotWeaponComponent(
    std::shared_ptr<IRandomNumberSource<int>> randSource,
    float numBullets
)
	: randSource(randSource), numBullets(numBullets)
{}

void RandomShotWeaponComponent::Fire(sf::Vector2f position, BulletConfig& config)
{
	for (float i = 0; i < numBullets; i++)
	{
		auto theta = AngleConversion::ToRadians((float)randSource->Generate(0, 360));
		auto speed = config.speed * (float)randSource->Generate(50, 250) / 100;
		sf::Vector2f arcVelocity(std::cos(theta) * (float)config.affinity, std::sin(theta));
		auto traj = BulletTrajectory(position, arcVelocity, speed);
		this->bulletSystem->FireBullet(this->bulletFactory, traj, config);
	}
}

WeaponState RandomShotWeaponComponent::getWeaponState() const
{
    return {
        "RandomShot",
        1.0,
        0.0,
        true
    };
}