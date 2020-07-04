#include "burst_shot_weapon_component.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

#include "../../../bullet/i_bullet_system.h"
#include "../../../bullet/bullet.h"

BurstShotWeaponComponent::BurstShotWeaponComponent(std::shared_ptr<IBulletSystem> bulletSystem, float delay, float arcAngle, float numBullets)
	: bulletSystem(bulletSystem), arcAngle(arcAngle * ((float)M_PI / 180.0f)), delay(delay), numBullets(numBullets), accumulator(0.0f)
{}

void BurstShotWeaponComponent::Fire(sf::Vector2f position, BulletConfig& config)
{
	this->accumulator += this->clockFire.restart().asSeconds();
	if (this->accumulator >= this->delay)
	{
		this->accumulator -= this->delay;

		// burst center point is (360 - theta) / 2
		float thetaStart = (((float)M_PI * 2.0f) - arcAngle) / 2.0f;
		float thetaEnd = thetaStart + arcAngle;

		for (float theta = thetaStart; theta < thetaEnd; theta += arcAngle / numBullets)
		{
			sf::Vector2f arcVelocity(config.speed * std::cos(theta), config.speed * std::sin(theta));
			this->bulletSystem->FireBullet(position, -arcVelocity, config);
		}
	}
}