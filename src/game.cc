#include <SFML/Graphics.hpp>
#include <chrono>
#include <algorithm>

#include "game.h"

#include "ui/fps.h"
#include "ui/player_hud.h"
#include "util/texture_atlas.h"
#include "renderer/glow_shader_renderer.h"
#include "renderer/composite_renderer.h"

#include "game_states/play/play_state_builder.h"
#include "game_states/play/play_state.h"
#include "game_states/play/ecs_play_state.h"
#include "game_states/menu/menu_state.h"

Game::Game()
	: clock(std::make_shared<sf::Clock>()),
	dt(1.0f / 60.0f),
	accumulator(0.0f)
{
	this->InitWindow();
	this->InitFps();
	this->InitTextureAtlas();

	this->InitGameStates();
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
		settings
    );

	auto view = this->window->getView();
	auto viewCenter = view.getCenter();
	auto viewSize = view.getSize();
	this->bounds = sf::FloatRect(
		viewCenter.x - viewSize.x / 2, // left
		viewCenter.y - viewSize.y / 2, // top
		viewSize.x,
		viewSize.y
    );

	auto glowRenderer = std::make_shared<GlowShaderRenderer>(viewSize);
	this->renderer = std::make_shared<CompositeRenderer>(glowRenderer, viewSize);
}

void Game::InitFps()
{
	this->fps = std::make_shared<Fps>();
}

void Game::InitTextureAtlas()
{
	this->textureAtlas =
		std::make_shared<TextureAtlas>()
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

void Game::InitGameStates()
{
	auto menuState = std::make_shared<MenuState>();

	// Legacy play state
	auto playState = std::make_shared<PlayState>(
		std::make_unique<PlayStateBuilder>(this->bounds, this->textureAtlas)
    );

	// NEW: ECS play state
	auto ecsPlayState = std::make_shared<ECSPlayState>(
		this->textureAtlas,
		this->bounds
	);

	// Set up transitions
	menuState->AddTransition(GameStates::PLAY, playState);
	menuState->AddTransition(GameStates::ECS_PLAY, ecsPlayState);
	playState->AddTransition(GameStates::MENU, menuState);
	ecsPlayState->AddTransition(GameStates::MENU, menuState);

	this->state = menuState;
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

	this->state = this->state->Yield();
}

void Game::Update()
{
	this->accumulator += this->clock->restart().asSeconds();
	while (this->accumulator >= this->dt)
	{
		this->state->Update(dt);
		this->fps->Update();
		this->accumulator -= this->dt;
	}
}

void Game::Draw()
{
	auto interp = this->accumulator / this->dt;

	auto bgColor = sf::Color(10, 0, 10);
	this->window->clear(bgColor);
	this->renderer->Clear();

	this->state->Draw(this->renderer, interp);

	this->fps->Draw(this->renderer);
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
