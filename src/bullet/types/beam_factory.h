#ifndef BEAM_FACTORY_H
#define BEAM_FACTORY_H

#include <SFML/Graphics.hpp>

#include "bullet/i_bullet_factory.h"
#include "beam.h"
#include "bullet/bullet.h"
#include "util/i_ray_caster.h"

class BeamFactory : public IBulletFactory
{
public:
	BeamFactory(std::shared_ptr<IRayCaster> rayCaster, sf::FloatRect bounds, float damageRate)
		: rayCaster(rayCaster), bounds(bounds), damageRate(damageRate)
	{}

	~BeamFactory() override = default;

	std::shared_ptr<Bullet> Construct(BulletTrajectory& trajectory, BulletConfig& config) const override {
		return std::make_shared<Beam>(trajectory, config, rayCaster, bounds, damageRate);
	};

private:
	std::shared_ptr<IRayCaster> rayCaster;
	sf::FloatRect bounds;
	float damageRate;
};

#endif //BEAM_FACTORY_H