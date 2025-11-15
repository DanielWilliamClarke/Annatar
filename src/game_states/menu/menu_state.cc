#include "menu_state.h"


#include <iostream>

#include "renderer/i_renderer.h"

MenuState::MenuState()
{
	font.loadFromFile("./assets/EightBitDragon-anqx.ttf");
	text.setFont(font);
	text.setScale(2, 2);
	text.setFillColor(sf::Color::Cyan);
	text.setPosition(50.0f, 50.0f);
	text.setString("PRESS RETURN FOR LEGACY MODE\n\nPRESS E FOR ECS MODE");
}

void MenuState::Setup()
{
	std::cout << "menu setting up" << std::endl;
}

void MenuState::TearDown()
{
	std::cout << "menu tearing down" << std::endl;
}

void MenuState::Update(float dt)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
	{
		return this->Forward(GameStates::PLAY);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
	{
		return this->Forward(GameStates::ECS_PLAY);
	}
}

void MenuState::Draw(const std::shared_ptr<IRenderer>& renderer, float interp) const
{
	renderer->GetTarget().draw(text);
}
