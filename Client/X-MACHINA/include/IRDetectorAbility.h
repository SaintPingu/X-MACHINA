#pragma once

#include "AbilityMgr.h"

class GameObject;

class IRDetectorAbility : public RenderedAbility {
	using base = RenderedAbility;

public:
	IRDetectorAbility(float cooldownTime, float activeTime);
};
