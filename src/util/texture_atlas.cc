#include "texture_atlas.h"
#include <iostream>

std::shared_ptr<ITextureAtlas> TextureAtlas::AddTexture(const std::string& tag, const std::string& texturePath)
{
	sf::Image image;
	if (!image.loadFromFile(texturePath)) {
		std::cerr << "ERROR: Failed to load texture: " << texturePath << std::endl;
		// Create a fallback pink texture so game doesn't crash
		image.create(32, 32, sf::Color::Magenta);
	}

	auto backgroundColor = image.getPixel(0, 0);
	image.createMaskFromColor(backgroundColor);

	auto texture = std::make_shared<sf::Texture>();
	texture->loadFromImage(image);

	this->textures[tag] = texture;
	return shared_from_this();
}

std::shared_ptr<sf::Texture> TextureAtlas::GetTexture(const std::string& tag) const
{
	return this->textures.at(tag);
}