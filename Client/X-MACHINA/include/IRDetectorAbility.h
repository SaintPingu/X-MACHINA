#pragma once

#include "AbilityMgr.h"

class GameObject;

class IRDetectorAbility : public RenderedAbility {
	using base = RenderedAbility;

public:
	IRDetectorAbility();

public:
	virtual bool Activate() override;
	virtual void DeActivate() override;
};
