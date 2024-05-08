#pragma once

#include "AbilityMgr.h"

class GameObject;

class IRDetectorAbility : public RenderedAbility {
	using base = RenderedAbility;

public:
	IRDetectorAbility(float cooldownTime, float activeTime);
	
public:
	virtual void Activate() override;
	virtual void DeActivate() override;
};
