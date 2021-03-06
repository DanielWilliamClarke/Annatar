#ifndef I_PLAYER_ATTRBUTE_COMPONENT_H
#define I_PLAYER_ATTRBUTE_COMPONENT_H
#pragma once

#include "i_attribute_component.h"

class IPlayerAttributeComponent: public IAttributeComponent
{
public:
	IPlayerAttributeComponent() = default;
	virtual ~IPlayerAttributeComponent() = default;
	virtual void Update(float dt) = 0;
};

#endif // I_ATTRBUTE_COMPONENT_H
