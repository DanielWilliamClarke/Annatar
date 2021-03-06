#include <SFML/Graphics.hpp>
#include <chrono>
#include <algorithm>

#include "game.h"

#include "ui/fps.h"
#include "ui/player_hud.h"

#include "util/texture_atlas.h"
#include "util/ray_caster.h"
#include "renderer/glow_shader_renderer.h"
#include "renderer/composite_renderer.h"

#include "entity/Entity.h"
#include "player/player_builder.h"
#include "player/player.h"
#include "player/player_input.h"

#include "enemy/enemy_system.h"
#include "enemy/enemy_type_factory.h"

#include "util/threaded_workload.h"
#include "util/random_number_mersenne_source.cc"
#include "level/space_level.h"

#include "components/movement/enemy_movement_component.h"
#include "components/movement/player_movement_component.h"
#include "components/movement/offset_movement_component.h"

#include "components/attributes/player_attribute_component.h"

#include "bullet/bullet_system.h"
#include "bullet/bullet.h"
#include "bullet/types/projectile_factory.h"
#include "bullet/types/beam_factory.h"

#include "components/weapon/burst/random_shot_weapon_component.h"
#include "components/weapon/burst/burst_shot_weapon_component_factory.h"
#include "components/weapon/single/single_shot_weapon_component_factory.h"
#include "components/weapon/beam/radial_beam_weapon_component_factory.h"

#include "components/animation/animation_component.h"
#include "components/hitbox/hitbox_component.h"
#include <bullet/types/homing_projectile_factory.h>
#include "bullet/types/debris_factory.h"

#include "quad_tree/quad_tree.h"

Game::Game()
	: clock(std::make_shared<sf::Clock>()),
	dt(1.0f / 60.0f),
	accumulator(0.0f),
	worldSpeed(40.0f)
{
	this->InitWindow();
	this->InitFps();
	this->InitTextureAtlas();
	this->InitLevel();
	this->InitBulletSystem();
	this->InitPlayer();
	this->InitEnemySystem();
}

void Game::InitWindow()
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	this->window = std::make_shared<sf::RenderWindow>(
		sf::VideoMode(1280, 720),
		//sf::VideoMode::getFullscreenModes()[0],
		"Space Shooter",
		sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close,
		settings);

	auto& view = this->window->getView();
	sf::Vector2f viewCenter(view.getCenter());
	sf::Vector2f viewSize(view.getSize());
	bounds = sf::FloatRect(viewCenter.x - viewSize.x / 2, // left
		viewCenter.y - viewSize.y / 2, // top
		viewSize.x,
		viewSize.y);

	auto glowRenderer = std::make_shared<GlowShaderRenderer>(viewSize);
	this->renderer = std::make_shared<CompositeRenderer>(glowRenderer, viewSize);

	this->threadableWorkload = std::make_shared<ThreadedWorkload>();
}

void Game::InitFps()
{
	this->fps = std::make_shared<Fps>();
}

void Game::InitTextureAtlas()
{
	this->textureAtlas = std::make_shared<TextureAtlas>();

	this->textureAtlas
		->AddTexture("playerShip", "assets/viperFrames.png")
		->AddTexture("playerExhaust", "assets/viperExhaust.png")
		->AddTexture("playerTurret", "assets/viperTurret.png")
		->AddTexture("playerGlowie", "assets/glowie.png")
		->AddTexture("enemy1", "assets/enemy_1.png")
		->AddTexture("enemy2", "assets/enemy_2.png")
		->AddTexture("enemy3", "assets/enemy_3.png")
		->AddTexture("enemy4", "assets/enemy_4.png")
		->AddTexture("boss1", "assets/boss_1.png")
		->AddTexture("big_core_mk_ii", "assets/bosses/big_core_mk_iii.png");
}

void Game::InitLevel()
{
	auto seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
	auto randGenerator = std::make_shared<RandomNumberMersenneSource<int>>(seed);
	this->level = std::make_shared<SpaceLevel>(std::make_shared<ThreadedWorkload>(), randGenerator, this->window->getView().getSize());
}

void Game::InitBulletSystem()
{
	this->bulletSystem = std::make_shared<BulletSystem>(bounds);

	auto factory = std::make_shared<DebrisFactory>();
	auto seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
	auto randGenerator = std::make_shared<RandomNumberMersenneSource<int>>(seed);
	this->debrisGenerator = std::make_shared<RandomShotWeaponComponent>(this->bulletSystem, factory, randGenerator, 5.0f);
}

void Game::InitPlayer()
{
	auto playerBuilder = std::make_shared<PlayerBuilder>(this->textureAtlas, this->bulletSystem, this->bounds);
	auto movementComponent = std::make_shared<PlayerMovementComponent>(this->bounds, this->worldSpeed);

	this->playerHud = std::make_shared<PlayerHud>(this->bounds);

	auto healthDamageColor = sf::Color(248, 99, 0, 255);
	auto sheildDamageColor = sf::Color(75, 108, 183, 255);
	auto attenuation = 50.0f;

	auto playerDamageEffects = std::make_shared<DamageEffects>(
		this->debrisGenerator,
		std::make_shared<BulletConfig>(nullptr,
			[=]() -> std::shared_ptr<sf::Shape> { return std::make_shared<sf::CircleShape>(2.0f, 3); },
			healthDamageColor, attenuation / 2, 0.0f, 20.0f, AFFINITY::RIGHT, false, 0.0f, 7.0f),
		std::make_shared<BulletConfig>(nullptr,
			[=]() -> std::shared_ptr<sf::Shape> { return std::make_shared<sf::CircleShape>(0.0f, 3); },
			healthDamageColor, attenuation, 0.0f, 50.0f, AFFINITY::RIGHT, false, 0.0f, 0.3f),
		std::make_shared<BulletConfig>(nullptr,
			[=]() -> std::shared_ptr<sf::Shape> { return std::make_shared<sf::CircleShape>(0.0f, 3); },
			sheildDamageColor, attenuation, 0.0f,  50.0f, AFFINITY::RIGHT, false, 0.0f, 0.3f));

	auto attributeComponent = std::make_shared<PlayerAttributeComponent>(this->playerHud, playerDamageEffects, PlayerAttributeConfig(100.0f, 50.0f, 10.0f, 3.0f));

	this->player = std::make_shared<Player>(playerBuilder, movementComponent, attributeComponent);
	this->playerTargets.push_back(this->player);
	this->playerInput = std::make_shared<PlayerInput>();
}

void Game::InitEnemySystem()
{
	this->enemySystem = std::make_shared<EnemySystem>();

	auto projectileFactory = std::make_shared<ProjectileFactory>();
	auto homingProjectileFactory = std::make_shared<HomingProjectileFactory>();
	auto beamFactory = std::make_shared<BeamFactory>(std::make_shared<RayCaster>(), this->bounds, 0.1f);

	auto healthDamageColor = sf::Color(248, 99, 0, 255);
	auto attenuation = 50.0f;

	auto enemyDamageEffects = std::make_shared<DamageEffects>(
		this->debrisGenerator,
		std::make_shared<BulletConfig>(nullptr,
			[=]() -> std::shared_ptr<sf::Shape> { return std::make_shared<sf::CircleShape>(2.0f, 3); },
			healthDamageColor, attenuation / 2, 0.0f, 20.0f, AFFINITY::LEFT, false, 0.0f, 0.7f),
		std::make_shared<BulletConfig>(nullptr,
			[=]() -> std::shared_ptr<sf::Shape> { return std::make_shared<sf::CircleShape>(0.0f, 3); },
			healthDamageColor, attenuation, 0.0f, 50.0f, AFFINITY::LEFT, false, 0.0f, 0.3f),
		nullptr);

	this->enemySystem

		->AddFactory(1.0f, std::make_shared<EnemyTypeFactory>(
			EnemyConfig(EnemyTypeFactory::BuildOribitalEnemy,
				EnemyMotionConfig(bounds, worldSpeed, 200.0f),
				EnemyAnimationConfig(this->textureAtlas->GetTexture("enemy1"), 6, 0.1f, 1.0f),
				EnemyWeaponConfig(std::make_shared<SingleShotWeaponComponentFactory>(projectileFactory), this->bulletSystem, 3.0f),
				EnemyAttributeConfig(enemyDamageEffects, 20.0f, 0.0f))))

		->AddFactory(2.0f, std::make_shared<EnemyTypeFactory>(
			EnemyConfig(EnemyTypeFactory::BuildLinearEnemy,
				EnemyMotionConfig(bounds, worldSpeed, 300.0f),
				EnemyAnimationConfig(this->textureAtlas->GetTexture("enemy2"), 14, 0.1f, 1.0f),
				EnemyWeaponConfig(std::make_shared<SingleShotWeaponComponentFactory>(homingProjectileFactory), this->bulletSystem, 3.0f),
				EnemyAttributeConfig(enemyDamageEffects, 40.0f, 0.0f))))

		->AddFactory(4.0f, std::make_shared<EnemyTypeFactory>(
			EnemyConfig(EnemyTypeFactory::BuildLinearEnemy,
				EnemyMotionConfig(bounds, worldSpeed, 75.0f),
				EnemyAnimationConfig(this->textureAtlas->GetTexture("enemy3"), 9, 0.1f, 1.0f),
				EnemyWeaponConfig(std::make_shared<BurstShotWeaponComponentFactory>(projectileFactory, 7.0f, 45.0f), this->bulletSystem, 4.0f),
				EnemyAttributeConfig(enemyDamageEffects, 60.0f, 0.0f))))

		->AddFactory(6.0f, std::make_shared<EnemyTypeFactory>(
			EnemyConfig(EnemyTypeFactory::BuildLinearEnemy,
				EnemyMotionConfig(bounds, worldSpeed, 100.0f),
				EnemyAnimationConfig(this->textureAtlas->GetTexture("enemy4"), 4, 0.1f, 1.0f),
				EnemyWeaponConfig(std::make_shared<BurstShotWeaponComponentFactory>(projectileFactory, 8.0f, 360.0f), this->bulletSystem, 4.0f),
				EnemyAttributeConfig(enemyDamageEffects, 30.0f, 0.0f))))

		->AddFactory(15.0f, std::make_shared<EnemyTypeFactory>(
			EnemyConfig(EnemyTypeFactory::BuildLinearEnemy,
				EnemyMotionConfig(bounds, worldSpeed, 75.0f),
				EnemyAnimationConfig(this->textureAtlas->GetTexture("boss1"), 12, 0.5f, 2.0f),
				EnemyWeaponConfig(std::make_shared<RadialBeamWeaponComponentFactory>(beamFactory, 0.5f, 10.0f, 3.0f), this->bulletSystem, 10.0f),
				EnemyAttributeConfig(enemyDamageEffects, 150.0f, 0.0f))));
}

void Game::WindowEvents()
{
	sf::Event event;
	while (this->window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			this->window->close();
		}
	}
}

void Game::Update()
{
	auto in = this->playerInput->SampleInput();
	this->accumulator += this->clock->restart().asSeconds();

	while (this->accumulator >= this->dt)
	{
		this->quadTree = std::make_shared<CollisionQuadTree>(bounds, 4);

		this->level->Update(worldSpeed, dt);
		this->player->Update(this->quadTree, in, this->dt);
		this->enemySystem->Update(this->quadTree, dt);
		this->bulletSystem->Update(this->quadTree, dt, worldSpeed);

		this->fps->Update();
		this->accumulator -= this->dt;
	}

	if (this->player->HasDied()) 
	{
		exit(0);
	}
}

void Game::Draw()
{
	auto interp = this->accumulator / this->dt;

	auto bgColor = sf::Color(10, 0, 10);
	this->window->clear(bgColor);
	this->renderer->Clear();

	this->level->Draw(this->renderer);
	this->bulletSystem->Draw(this->renderer, interp);
	this->player->Draw(this->renderer, interp);
	this->enemySystem->Draw(this->renderer, interp);
	this->fps->Draw(this->renderer);
	this->playerHud->Draw(this->renderer);
	this->quadTree->Draw(this->renderer);

	this->renderer->Draw(*this->window);
	this->window->display();
}

void Game::Run()
{
	while (this->window->isOpen())
	{
		this->WindowEvents();
		this->Update();
		this->Draw();
	}
}
