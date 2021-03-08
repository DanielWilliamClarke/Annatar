#include "radial_beam_weapon_component.h"

#include "util/math_utils.h"

#include "bullet/i_bullet_system.h"
#include "bullet/i_bullet_system.h"
#include "bullet/bullet.h"
#include "bullet/types/beam.h"

RadialBeamWeaponComponent::RadialBeamWeaponComponent(std::shared_ptr<IBulletSystem> bulletSystem, std::shared_ptr<IBulletFactory> factory, float duration, float coolDown, float arcAngle, float numBeams)
	: bulletSystem(bulletSystem),
	factory(factory),
	arcAngle(AngleConversion::ToRadians(arcAngle)),
	duration(duration),
	coolDown(coolDown),
	numBeams(numBeams),
	accumulator(0.0f)
{
}

void RadialBeamWeaponComponent::Fire(sf::Vector2f position, std::shared_ptr<BulletConfig> config)
{
	if (!beams.size())
	{
		// burst center point is (360 - theta) / 2
		float theta = (((float)M_PI * 2.0f) - arcAngle) / 2.0f;
		for (float i = 0; i < numBeams; i++)
		{
			sf::Vector2f arcVelocity(std::cos(theta) * (float)config->affinity, std::sin(theta));
			auto traj = BulletTrajectory(position, -arcVelocity, config->speed);

			auto beam = std::dynamic_pointer_cast<Beam>(
				this->bulletSystem->FireBullet(factory, traj, config));

			beams.push_back(beam);
			theta += arcAngle / numBeams;
		}
	}

	// Reigite all beams if still within duration
	this->accumulator += this->clock.restart().asSeconds();
	if (this->accumulator < this->duration)
	{
		for (auto beam : beams)
		{
			if (!beam->isSpent())
			{
				beam->Reignite();
			}
		}
		return;
	}

	// Clear any spent beams
	this->beams.erase(
		std::remove_if(
			this->beams.begin(), this->beams.end(),
			[=](std::shared_ptr<Beam>& b) -> bool {
				return b->isSpent();
			}),
		this->beams.end());

	if (!beams.size() && this->accumulator > this->duration + config->lifeTime + this->coolDown)
	{
		this->accumulator = 0;
	}
}