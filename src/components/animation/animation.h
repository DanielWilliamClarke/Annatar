#ifndef ANIMATION_H
#define ANIMATION_H

#include <SFML/Graphics.hpp>
#include <memory>

class Animation
{
public:
	Animation(std::shared_ptr<sf::Sprite> sprite, std::shared_ptr<sf::Texture> textureSheet,
		float frameDuration, int startFrameX, int startFrameY, int framesX, int framesY, int width, int height);

	virtual ~Animation() = default;

	[[nodiscard]] bool IsDone() const;
    bool Play(bool loop = true);
    bool Play(float modPercent, bool loop = true);
	void Reset();

private:
	void NextFrame(bool loop);

	//Variables
	std::shared_ptr<sf::Sprite> sprite;
	std::shared_ptr<sf::Texture> textureSheet;

	int width;
	int height;
	sf::IntRect startRect;
	sf::IntRect currentRect;
	sf::IntRect endRect;

	float frameDuration;
	sf::Clock clockAnimate;
	bool done;
};

#endif