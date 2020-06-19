#pragma once

#include "i_animation_component_factory.h"
#include "animation_component.h"

class AnimationComponentFactory : public IAnimationComponentFactory
{
public:
	AnimationComponentFactory() = default;
	virtual ~AnimationComponentFactory() = default;
	virtual std::shared_ptr<AnimationComponent> CreateInstance() {
		return std::make_shared<AnimationComponent>();
	}
};