#ifndef ENEMY_SYSTEM_H
#define ENEMY_SYSTEM_H
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <map>

class Enemy;
class IEnemyTypeFactory;

class EnemySystem: public std::enable_shared_from_this<EnemySystem>
{
public:
	EnemySystem();
	virtual ~EnemySystem() = default;

	void Update(float dt);
	void Draw(sf::RenderTarget& target, float interp) const;

	virtual std::shared_ptr<EnemySystem> AddFactory(float spawnInterval, std::shared_ptr<IEnemyTypeFactory> factory);

private:
	std::vector<std::shared_ptr<Enemy>> enemies;
	std::map<float, std::shared_ptr<IEnemyTypeFactory>> factories;

	sf::Clock spawnClock;
	float accumulator;
	float maxInterval;
};

#endif //ENEMY_SYSTEM_H